/*
 * Copyright 2013 BOUVIER-VOLAILLE Julien
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "cjson_dictionary.h"

#include <stdio.h>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <vector>
#include "utils.h"

#include <fields/extended/cjson_extended_field.h>
#include <fields/cjson_array_field.h>

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <dlfcn.h>

#include "demangle.h"

#include <fields/cjson_fundamental_field.h>
#include <fields/cjson_method_field.h>
#include <fields/cjson_recursive_field.h>

using namespace xercesc;
using namespace std;


// Redefining Memory Allocation 

static void* (*real_malloc)(size_t)=NULL;
static void* (*real_calloc)(size_t)=NULL;
static void* (*real_realloc)(void*, size_t)=NULL;
static void  (*real_free)(void *ptr);


static void mtrace_init(void)
{
    real_malloc = (void* (*)(size_t)) dlsym(RTLD_NEXT, "malloc");
    if (NULL == real_malloc) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
    // a bit slower than expected - here calloc not used
    real_calloc = (void* (*)(size_t)) dlsym(RTLD_NEXT, "calloc");
    if (NULL == real_calloc) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }

    real_realloc = (void* (*)(void*, size_t)) dlsym(RTLD_NEXT, "realloc");
    if (NULL == real_realloc) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }	
	
    real_free = (void (*)(void*)) dlsym(RTLD_NEXT, "free");
    if (NULL == real_free) {
        fprintf(stderr, "Error in `dlsym`: %s\n", dlerror());
    }
}

void *calloc(size_t n, size_t size)
{
	if(real_malloc==NULL) {
	   return NULL; // hack see http://code.google.com/p/chromium/issues/detail?id=28244
        //mtrace_init();
     }
	size_t total = n * size ; // can be hacked if mult overflow
	if (total==0) return NULL;
	total += ENV_SIZE_BYTES;

	void *p = real_malloc(total); 
	memset(p, 0, total);

	((long*) p)[0] = total-ENV_SIZE_BYTES; // Save size of allocated 
     return (void*) ((unsigned long) p+ENV_SIZE_BYTES);
}


void *malloc(size_t size)
{
    if(real_malloc==NULL) {
        mtrace_init();
    }

    void *p = NULL;
    //fprintf(stderr, "malloc(%d) = ", size);

    p = real_malloc(size+ENV_SIZE_BYTES); // Allocate more size than expected
    memset(p,0,size);

    //fprintf(stderr, "%p\n", p);
    ((long*) p)[0] = size; // Save size of allocated 
    return (void*) ((unsigned long) p+ENV_SIZE_BYTES);
}

void *realloc(void *ptr, size_t size)
{
    if(real_malloc==NULL) {
        mtrace_init();
    }

    if (ptr==0) {
		return malloc(size);
    }
		
    void* newPtr = real_realloc((void*) ((long) ptr-8),size+8);
    return (void*) ((long) newPtr+8); 
}

void free(void *ptr)
{
    // something wrong if we call free before one of the allocators!
    if (ptr==0) return;
    //long aSize = *((long*)((unsigned long) ptr-8));	// Print deallocated size   
    //fprintf(stderr, "free(%d) = %p\n", aSize, (void*) ((unsigned long) ptr-8));
    real_free((void*) ((unsigned long) ptr-ENV_SIZE_BYTES));

}

void* operator new(size_t num)
{
    void* p = malloc(num);	
    //fprintf(stderr, "new(%d) = %p\n", num, p);
    return p;
}

void operator delete(void *ptr)
{
    free(ptr);
}

long pointer_size(void *ptr){
	return *((long*)((unsigned long) ptr-ENV_SIZE_BYTES));
}

// Starting dictionary definition

namespace cjson {

	void xmlErrorHandler::fatalError(const SAXParseException &exc) {
		   printf("Fatal parsing error at line %d\n",
		         (int)exc.getLineNumber());
		   exit(-1);
	}


	// Dictionnary definition

	dictionary::dictionary(std::string iFilename){

		m_parser = NULL;
		m_errHandler = NULL;

		_filename = iFilename;

		init();

	}

	dictionary::~dictionary(){


		if (m_parser) {
		   delete m_parser;
		   delete m_errHandler;
		   XMLPlatformUtils::Terminate();
		}

		
		std::map<std::string, field::field* >::iterator iter;		
		for (iter = _resources.begin(); iter != _resources.end(); ++iter) {
			if (iter->second) {
#ifdef __DEBUG__
				printf("delete %s %p\n",iter->second->getName().c_str(),iter->second);
#endif
				delete iter->second;
			} else {
#ifdef __DEBUG__
				printf("delete empty ? %s \n",iter->first.c_str());
#endif
			}
	   	}	

			
		for (iter = _fundamentals.begin(); iter != _fundamentals.end(); ++iter) {
#ifdef __DEBUG__
			printf("delete %s %p\n",iter->second->getName().c_str(),iter->second);
#endif
			delete iter->second;
	   	}
			

	}

	

	void dictionary::init(){

	   XMLPlatformUtils::Initialize();
	   m_parser = new XercesDOMParser();
	   m_errHandler = new xmlErrorHandler();
	   m_parser->setErrorHandler(m_errHandler);

    	   m_parser->parse(_filename.c_str());
    		
        _document = m_parser->adoptDocument();

	   recordStructures(_document);

	   dl_iterate_phdr(retrieve_symbolnames, &_symbolVTableAddresses); // We can find vtable addresses	

	   for (map<string, field::field*>::iterator it=_resources.begin(); it!=_resources.end(); ++it) {
    		
		   cjson::field::class_field* aFClass = dynamic_cast<cjson::field::class_field*>(it->second);
		   //printf("fill vtable address %s \n",it->first.c_str());
		   void* aVtableAddress = getVTableAddress(/*aFClassSP*/it->second->getName()); // ask vtable adress for the layout

		   if (aVtableAddress!=0) {
				if (aFClass) aFClass->getABIInst().buildVirtualTableLayout(aFClass->getVTableLayout(), aFClass->getVTableEntryPoint(), aVtableAddress);
		   } 

	   }

	}

	void* dictionary::getVTableAddress(std::string iClassName){

		string aKey = "vtable for "+iClassName;
		if (_symbolVTableAddresses.count( aKey )) {
			return _symbolVTableAddresses[aKey];
		}
		else return 0;	
	}
		
	// Here we need to read symbol tables
	bool dictionary::startsWith(const char *pre, const char *str)
	{
	    size_t lenpre = strlen(pre),
		      lenstr = strlen(str);
	    return lenstr < lenpre ? false : strncmp(pre, str, lenpre) == 0;
	}

#ifndef ELF64_ST_BIND
	#define ELF64_ST_BIND(i)   ((i)>>4)
#endif
#ifndef ELF64_ST_TYPE
	#define ELF64_ST_TYPE(i)   ((i)&0xf)
#endif
#ifndef ELF64_ST_VISIBILITY
	#define ELF64_ST_VISIBILITY(o) ((o)&0x3)
#endif

	/* Callback for dl_iterate_phdr.
	 * Is called by dl_iterate_phdr for every loaded shared lib until something
	 * else than 0 is returned by one call of this function.
	 */
	int dictionary::retrieve_symbolnames(struct dl_phdr_info* info, size_t info_size, void* symbol_names_map) 
	{

	    /* ElfW is a macro that creates proper typenames for the used system architecture
		* (e.g. on a 32 bit system, ElfW(Dyn*) becomes "Elf32_Dyn*") */
	    ElfW(Dyn*) dyn;
	    ElfW(Sym*) sym;
	    ElfW(Word*) hash;

	    char* strtab;
	    char* sym_name;
	    ElfW(Word) sym_cnt;

	    /* the void pointer (3rd argument) should be a pointer to a vector<string>
		* in this example -> cast it to make it usable */
	    map<std::string,void*>* symbol_names = reinterpret_cast< map<std::string,void*>*>(symbol_names_map);

	    /* Iterate over all headers of the current shared lib
		* (first call is for the executable itself) */
	    for (size_t header_index = 0; header_index < info->dlpi_phnum; header_index++)
	    {

		   if (info->dlpi_phdr[header_index].p_type == PT_LOAD) {
#ifdef __DEBUG__
			  printf("PT_LOAD %s\n",info->dlpi_name);
#endif
		   }	
		   /* Further processing is only needed if the dynamic section is reached */
		   else if (info->dlpi_phdr[header_index].p_type == PT_DYNAMIC)
		   {

			  /* Get a pointer to the first entry of the dynamic section.
		        * It's address is the shared lib's address + the virtual address */
		       dyn = (ElfW(Dyn)*)(info->dlpi_addr +  info->dlpi_phdr[header_index].p_vaddr);
#ifdef __DEBUG__
			  printf("PT_DYNAMIC Segment %s (%d) [%p]\n",info->dlpi_name,(int) header_index,dyn);		
#endif
			  if (dyn!=0) {

				 

				  /* Iterate over all entries of the dynamic section until the
				   * end of the symbol table is reached. This is indicated by
				   * an entry with d_tag == DT_NULL.
				   *
				   * Only the following entries need to be processed to find the
				   * symbol names:
				   *  - DT_HASH   -> second word of the hash is the number of symbols
				   *  - DT_STRTAB -> pointer to the beginning of a string table that
				   *                 contains the symbol names
				   *  - DT_SYMTAB -> pointer to the beginning of the symbols table
				   */
				  while(dyn->d_tag != DT_NULL)
				  {
#ifdef __DEBUG__
					 printf("Tags [%d] [%8x] \n",(int) dyn->d_tag, (int)dyn->d_tag);
#endif
				      if (dyn->d_tag == DT_HASH)
				      {
				          /* Get a pointer to the hash */
				          int* hash =  (int*) dyn->d_un.d_ptr ;
#ifdef __DEBUG__
						printf("Number of symbs [%d] \n",hash[1]);
#endif
				          /* The 2nd word is the number of symbols */
				          sym_cnt = hash[1];

				      } 
					 else if (dyn->d_tag == DT_GNU_HASH ) {
#ifdef __DEBUG__
						printf("GNU Hash\n");
#endif

						void* hash = (void*) dyn->d_un.d_ptr;
						unsigned int nbuckets = *((int*) hash);
#ifdef __DEBUG__
						printf("nbuckets %d\n",nbuckets);
#endif
						unsigned int symbol_base = ((int*) hash)[1];
#ifdef __DEBUG__
						printf("symbol_base %d\n",symbol_base);
#endif
						unsigned int bitmask_nwords = ((int*) hash)[2];
#ifdef __DEBUG__
						printf("bitmask_nwords %d\n",bitmask_nwords);
#endif

						unsigned int* bucket = (unsigned int*) ((unsigned long) hash + 16 + bitmask_nwords * ENV_SIZE_BYTES); // (4 32Bits or 8 on x64)
						unsigned int last_sym = 0;
						for (int index = 0; index < nbuckets; index++) {
						    if (bucket[index] > last_sym)
							    last_sym = bucket[index];      
						}
#ifdef __DEBUG__
						printf("last_sym %d\n",last_sym);
#endif
						sym_cnt = last_sym;	

					 }
				      else if (dyn->d_tag == DT_STRTAB)
				      {
				          /* Get the pointer to the string table */
				          strtab = (char*)dyn->d_un.d_ptr;
				      }
				      else if (dyn->d_tag == DT_SYMTAB)
				      {

				          /* Get the pointer to the first entry of the symbol table */
				          sym = (ElfW(Sym*))dyn->d_un.d_ptr;
#ifdef __DEBUG__
						printf("sym [%p] \n",sym);	
#endif
				          /* Iterate over the symbol table */
				          for (ElfW(Word) sym_index = 0; sym_index < sym_cnt; sym_index++)
				          {
				              /* get the name of the i-th symbol.
				               * This is located at the address of st_name
				               * relative to the beginning of the string table. */
				              sym_name = &strtab[sym[sym_index].st_name];

						    void* aAddress = (void*) sym[sym_index].st_value;

						    if (startsWith("_ZTVN", sym_name)) { // Select only vtables

								char aDemangled[2048];
								demangleFunctionPointer(aDemangled, 2048, (void*) aAddress);

						    		//printf("SYM(%d) [%p] index [%d] :%s %s \n",sym_index,aAddress, sym[sym_index].st_shndx , aDemangled, sym_name);	
								//printf("TYPE:%d\n",ELF64_ST_TYPE(sym[sym_index].st_info));
								//printf("BINDING:%d\n",ELF64_ST_BIND(sym[sym_index].st_info));
								//printf("VISIBILITY:%d\n",ELF64_ST_VISIBILITY(sym[sym_index].st_other));
				              		(*symbol_names)[string(aDemangled)] = aAddress;
						    }

				          }
				      }

				      /* move pointer to the next entry */
				      dyn++;
				  }

			  }

		   } else {
#ifdef __DEBUG__
			  printf("PT %s [%d]\n",info->dlpi_name,(int) header_index);
#endif
		   }	
		   
	    }

	    /* Returning something != 0 stops further iterations,
		* since only the first entry, which is the executable itself, is needed
		* 1 is returned after processing the first entry.
		*
		* If the symbols of all loaded dynamic libs shall be found,
		* the return value has to be changed to 0.
		*/
	    return 1;

	}

	// XML Char manipulation
	void dictionary::releaseFunction(XMLCh* p) { XMLString::release(&p); }

	void dictionary::str(std::string& ioString, const XMLCh* aName){
		char* temp2 = XMLString::transcode(aName);
          ioString = temp2;
          XMLString::release(&temp2);
	}

	smart_ptr<XMLCh> dictionary::str(const std::string& iStr){
		return smart_ptr<XMLCh>(XMLString::transcode(iStr.c_str()), releaseFunction );
	}

	smart_ptr<XMLCh> dictionary::operator=(const std::string& rhs) {
		return smart_ptr<XMLCh>(XMLString::transcode(rhs.c_str()), releaseFunction );
	}

	// Core decoding code
	void dictionary::recordStructures(DOMDocument* iDocument){

		DOMNodeList* aList = iDocument->getElementsByTagName(str("Class").get()); // In there we only decode classes

		for (int i=0;i<aList->getLength(); i++) {
#ifdef __DEBUG__
			printf("##########################################################################\n");
#endif
			DOMElement* aElement = dynamic_cast<DOMElement*>(aList->item(i));
			if (aElement) {

				std::vector<std::string> aRecursive;
				int aHasRecursive = 0; 
				field::field* aClass = decodeClassOrStruct(iDocument, aElement, aRecursive, aHasRecursive);
				if (aClass) {

					if (_resources.find(aClass->getName())==_resources.end()) {
						_resources[aClass->getName()] = aClass;
					}

				}

			}
		
		} 	

		aList = iDocument->getElementsByTagName(str("Struct").get()); // In there we only decode classes

		for (int i=0;i<aList->getLength(); i++) {
#ifdef __DEBUG__
			printf("##########################################################################\n");
#endif
			DOMElement* aElement = dynamic_cast<DOMElement*>(aList->item(i));
			if (aElement) {

				std::vector<std::string> aRecursive;
				int aHasRecursive = 0; 
				field::field* aClass = decodeClassOrStruct(iDocument, aElement, aRecursive, aHasRecursive);
				if (aClass) {

					if (_resources.find(aClass->getName())==_resources.end()) {
						_resources[aClass->getName()] = aClass;
					}

				}

			}
		
		} 		
	

	}

	void dictionary::removeAttr(const std::string iID, std::string& ioShorten){

		ioShorten = iID;
		if (iID.find(":") != std::string::npos) {
			std::vector<std::string> aItems;
			split(iID,':',aItems);
			ioShorten = aItems[1];
		}		

	}

	#define UNKNOWN     0
	#define CONSTRUCTOR 1
	#define DESTRUCTOR  2
	#define METHOD      3
	#define NOTMETHOD   4

	bool dictionary::haveRecursion(std::vector<std::string>& iRecursive, std::string& iName){
		return std::find(iRecursive.begin(), iRecursive.end(), iName)!=iRecursive.end();
	}

	field::field* dictionary::decodeClassOrStruct(DOMDocument* iDocument, DOMElement* iClass,  
								   		  std::vector<std::string>& iRecursive, int& iHasRecursive){

		std::string aStrName;
		str(aStrName, iClass->getAttribute(str("demangled").get()));

		if (_resources.find(aStrName)!=_resources.end()) {
			return _resources[aStrName];
		}		

#ifdef __DEBUG__
			printf("Starting class declaration %s\n",aStrName.c_str());
#endif
		

		if (haveRecursion(iRecursive, aStrName)) {
#ifdef __DEBUG__
			printf("found recursive declaration\n");
#endif
			iHasRecursive+=1; // can have  multiple recursion ? on multiple fields ?
			return new field::recursive_field(aStrName,this);
		}

		// Manage recursive
		iRecursive.push_back(aStrName);

		std::string aSize;
		str(aSize, iClass->getAttribute(str("size").get()));

		std::string aBases;
		str(aBases, iClass->getAttribute(str("bases").get()));
		
		std::vector<std::string> aBaseList;
		split(aBases,' ',aBaseList);

		field::extended_field* aExtField = field::ExtendedFieldFactory::getInstance().getField(aStrName,this);
			
		// create field object		
		field::class_field* apField = new field::class_field(aStrName,this) ;

		if (aExtField) {
			aExtField->setName(aStrName);
			aExtField->setRepField(apField);
		}

		apField->setSize(atoi(aSize.c_str()));

		apField->getABIInst()._name = aStrName;

		apField->getVTableOffsets()[aStrName] = 0;

		int aCurrentOffset = 0;
		if (aBaseList.size()>0) {

			DOMNodeList* aList = iClass->getElementsByTagName(str("Base").get()); // In there we only decode classes

			for (int i=0;i<aList->getLength(); i++) {
#ifdef __DEBUG__
				printf("##########################################################################\n");
#endif
				DOMElement* aBaseElement = dynamic_cast<DOMElement*>(aList->item(i));

			
				std::string aId;
				str(aId, aBaseElement->getAttribute(str("type").get()));

				std::string aBaseOffset;	
				str(aBaseOffset, aBaseElement->getAttribute(str("offset").get()));

				std::string aVirtual;	
				str(aVirtual, aBaseElement->getAttribute(str("virtual").get()));

				aCurrentOffset = atoi(aBaseOffset.c_str())*8; // (8 bit) here bug in xml file not in bit but in byte


				DOMElement* aElement =  iDocument->getElementById(str(aId).get());
#ifdef __DEBUG__
				printf("Current Offset: %d\n",aCurrentOffset);
				printf("Base id:#%s#%s# %p\n",aBaseList[i].c_str(), aId.c_str(), aElement);
#endif

				// Copy fields

				std::vector<std::string> aRecursive;
				int aHasRecursive = 0; 
				field::field* apBaseField = decodeClassOrStruct(iDocument, aElement, aRecursive, aHasRecursive);

				field::class_field* aClassSP = dynamic_cast<field::class_field*>(apBaseField);
#ifdef __DEBUG__
				printf("Current decoded string %s\n",apBaseField->getName().c_str());
				printf("<<<<##################################################\n");
#endif
				std::map<std::string ,field::attached_field* >* aList = &aClassSP->getList();
				std::map<std::string, field::attached_field* >::iterator iter;	
			
				//aClassSP->setOffset(aCurrentOffset);	
				apField->addInherited(aCurrentOffset, aClassSP);

				
		   		for (iter = aList->begin(); iter != aList->end(); ++iter) {
				
					field::attached_field* aField = iter->second;
					if (aField) {
						//aField->shiftOffset(aCurrentOffset);

#ifdef __DEBUG__						
						printf("External field : %s \n",iter->first.c_str());
#endif

						if (iter->first.find("::") != std::string::npos) {
							apField->addField(iter->first, aCurrentOffset+aField->getOffset() , aField->getField());
						} else { // TODO check this in case diamond graph
							apField->addField(aClassSP->getName()+"::"+iter->first, aCurrentOffset+aField->getOffset() , aField->getField());
						}						

					}
				
				}

				// Set offset of virtual classes (if they are) 
				//printf("recorded %s %d\n",aClassSP->getName().c_str(),aCurrentOffset);
				apField->getVTableOffsets()[aClassSP->getName()] = aCurrentOffset;

				ABI_Inh aInheritedABI = aClassSP->getABI();

				if (atoi(aVirtual.c_str())==1) {
					aInheritedABI._virtual_inherited = true;
				}

				apField->getABIInst()._inherited.push_back(aInheritedABI);

				//aCurrentOffset += apBaseField->getSize();

			}

		}

#ifdef __DEBUG__
		printf("Class:%s ", aStrName.c_str());
		printf(" Size:%s\n", aSize.c_str());
#endif

		std::string aMembers;
		str(aMembers, iClass->getAttribute(str("members").get()));
		
		std::vector<std::string> aMemberList;
		split(aMembers,' ',aMemberList);

		std::string aMemberName;
		for (int i=0; i<aMemberList.size(); i++) {
			int iMethod = 0;
			bool aVirtual = false;
			unsigned long aFieldOffset = 0;
			field::field* aField = decodeMember(iDocument, aMemberList[i], aFieldOffset, aMemberName, iMethod, aVirtual, iRecursive, iHasRecursive);
			if (iMethod==NOTMETHOD) {
				if (aField) {
					//aField->shiftOffset(0);
					apField->addField(aMemberName, aFieldOffset, aField);
				}			
			} else  {
				
				if (aVirtual) {

					if (iMethod==METHOD) {
						field::method_field* aMethodField = dynamic_cast<field::method_field*>(aField);	
						aMethodField->getABIInst()._class = apField->getName(); 				
						apField->getABIInst()._virtual_methods.push_back(aMethodField->getABIInst());
					}

				} else {

					delete aField;
					aField = 0;			
	
				}	
	
			}
		}

		if (apField->getListSize()==0) {
			apField->getABIInst()._empty = true;
		} else {
			apField->getABIInst()._empty = false;
		}

		if (aExtField) {
			// Manage recursive
			iRecursive.pop_back();
			return aExtField;
		}

		// Manage recursive
		iRecursive.pop_back();
		return apField;

	}

		
	field::field* dictionary::decodeMember(DOMDocument* iDocument, const std::string& iID, unsigned long& ioOffset, std::string& ioMemberName, int& iMethod, bool& ioVirtual, 
								     std::vector<std::string>& iRecursive, int& iHasRecursive){

		field::field* aField = NULL;	

		DOMElement* aElement =  iDocument->getElementById(str(iID).get());

		ioVirtual = false;

		if (aElement) {

			std::string aType;
			str(aType, aElement->getTagName());

			if (aType.compare("Field")==0) {
				iMethod = NOTMETHOD;
				aField = decodeField(iDocument, aElement, ioOffset, ioMemberName, iRecursive, iHasRecursive);
			} else if ((aType.compare("Method")==0) || (aType.compare("OperatorMethod")==0)){
				iMethod = METHOD;
				aField = decodeMethod(iDocument, aElement, ioMemberName, ioVirtual);
		     } else if (aType.compare("Destructor")==0) {
				iMethod = CONSTRUCTOR;
				aField = decodeDestructor(iDocument, aElement, ioMemberName, ioVirtual);
			} else if (aType.compare("Constructor")==0) {  
				iMethod = DESTRUCTOR;
				aField = decodeConstructor(iDocument, aElement, ioMemberName);
			} else if ((aType.compare("Struct")==0) || (aType.compare("Class")==0)) {  
				iMethod = NOTMETHOD;
#ifdef __DEBUG__
				printf("Inner defined ");
#endif
				// If we need them for an member we will read them anyway
				//aField = decodeClassOrStruct(iDocument, aElement, iRecursive, iHasRecursive);
				//if (aField) { // TODO implement recursive
				//	_resources[aField->getName()] = aField;
				//}

			}
#ifdef __DEBUG__
			 else {
				//printf("Undecoded type:%s\n",aType.c_str());
			}
#endif
		} else {
			printf("Element not found.\n");
		}	

		return aField;

	}

     field::field* dictionary::decodeConstructor(DOMDocument* iDocument, DOMElement* iField , std::string& ioMethodName) {

		std::string aName; //name
		str(aName, iField->getAttribute(str("name").get()));

		ioMethodName = aName;

		return NULL; //aField;

	}

	field::field* dictionary::decodeDestructor(DOMDocument* iDocument, DOMElement* iField , std::string& ioMethodName, bool& ioVirtual) {

		std::string aName; //name
		str(aName, iField->getAttribute(str("name").get()));
		std::string aVirtual; //type 
		str(aVirtual, iField->getAttribute(str("virtual").get()));

		ioMethodName = aName;

		if (atoi(aVirtual.c_str())==1) {
			ioVirtual = true;
		}

		return NULL; //aField;

	}

	field::field* dictionary::decodeMethod(DOMDocument* iDocument, DOMElement* iField , std::string& ioMethodName, bool& ioVirtual) {

		std::string aName; //name
		str(aName, iField->getAttribute(str("name").get()));
		std::string aMangled; //name
		str(aMangled, iField->getAttribute(str("mangled").get()));
		std::string aVirtual; //type 
		str(aVirtual, iField->getAttribute(str("virtual").get()));

		ioMethodName = aName;

		if (atoi(aVirtual.c_str())==1) {
			ioVirtual = true;
		}
		
		field::method_field* aField = new field::method_field(aMangled,aName,this);

		DOMNodeList* aList = iField->getElementsByTagName(str("Argument").get()); // In there we only decode classes

		for (int i=0;i<aList->getLength(); i++) {
#ifdef __DEBUG__
			//printf("##########################################################################\n");
#endif
			DOMElement* aArgElement = dynamic_cast<DOMElement*>(aList->item(i));

			std::string aType; // TODO here only the type is needed ... we can ask for more
			str(aType, iField->getAttribute(str("type").get()));

			aField->addArgument(aType); // this will add args to the ABI structure

		}

		return aField; //aField;

	}

     field::field* dictionary::decodeField(DOMDocument* iDocument, DOMElement* iField , unsigned long& ioOffset, std::string& ioMemberName, 
								    std::vector<std::string>& iRecursive, int& iHasRecursive) {

		std::string aName; //name
		str(aName, iField->getAttribute(str("name").get()));
		std::string aType; //type 
		str(aType, iField->getAttribute(str("type").get()));
		std::string aOffset; //offset
		str(aOffset, iField->getAttribute(str("offset").get()));

#ifdef __DEBUG__
		printf("()*-------------------------------------------------------------------*\n");
		printf("()Name:%s Offset:%4s \n",aName.c_str(),aOffset.c_str());
#endif

		ioMemberName = aName;
		ioOffset = atoi(aOffset.c_str());

		return decodeType(iDocument, aType, iRecursive, iHasRecursive);	
	
	}
	
	field::field* dictionary::decodeType(DOMDocument* iDocument, const std::string& iID, 
								   std::vector<std::string>& iRecursive, int& iHasRecursive) {

		DOMElement* aElement =  iDocument->getElementById(str(iID).get());

		if (aElement) {
			std::string aType;
			str(aType, aElement->getTagName());
			
			if (aType.compare("FundamentalType")==0) {
				return decodeFundamentalType(iDocument, aElement); 
			} else if (aType.compare("Typedef")==0) {
				return decodeTypeDef(iDocument, aElement,  iRecursive, iHasRecursive);
			} else if (aType.compare("Struct")==0) {
				return decodeClassOrStruct(iDocument, aElement,  iRecursive, iHasRecursive);
			} else if (aType.compare("PointerType")==0) {
				return decodePointerType(iDocument, aElement,  iRecursive, iHasRecursive);
			} else if (aType.compare("Class")==0) {
				return decodeClassOrStruct(iDocument, aElement, iRecursive, iHasRecursive);
			} else if (aType.compare("ArrayType")==0) {
				return decodeArrayType(iDocument, aElement,  iRecursive, iHasRecursive);
			} else if (aType.compare("Enumeration")==0) {
				return decodeEnumeration(iDocument, aElement);	
			}
#ifdef __DEBUG__
			else {
				printf("Unable to decode %s\n",aType.c_str());
			}
#endif

		}

		return NULL;

	}

	field::field* dictionary::decodeArrayType(DOMDocument* iDocument, DOMElement* iField, 
								        std::vector<std::string>& iRecursive, int& iHasRecursive){

		std::string aSize; //size 
		str(aSize, iField->getAttribute(str("size").get()));
		std::string aType; //type
		str(aType, iField->getAttribute(str("type").get()));

		std::string aMin; //min
		str(aMin, iField->getAttribute(str("min").get()));

		std::string aMax; //max
		str(aMax, iField->getAttribute(str("max").get()));
		
		field::field* aField = decodeType(iDocument, aType, iRecursive, iHasRecursive);

		if (aField) {
  			return new field::array_field("Array",atoi(aSize.c_str()),atoi(aMax.c_str())-atoi(aMin.c_str())+1,aField,this);
		} 		
		return NULL;	 
		
	}

	field::field* dictionary::decodeEnumeration(DOMDocument* iDocument, DOMElement* iField){

		std::string aSize; //size 
		str(aSize, iField->getAttribute(str("size").get()));

		std::string aAlign; //align
		str(aAlign, iField->getAttribute(str("align").get()));

		int size = atoi(aSize.c_str());

		if (size==32) {
			return field::FundamentalFactory::getInstance().getField("int", atoi(aAlign.c_str()), this);
		} 

		// TODO insert for 64bits version

		return NULL;

	}

	field::field* dictionary::decodeFundamentalType(DOMDocument* iDocument, DOMElement* iField){

		std::string aName; //name
		str(aName, iField->getAttribute(str("name").get()));

		if (_fundamentals.find(aName)!=_fundamentals.end()) {
			return _fundamentals[aName];
		}

		field::field* aField;
		
		std::string aSize; //size 
		str(aSize, iField->getAttribute(str("size").get()));
		std::string aAlign; //align
		str(aAlign, iField->getAttribute(str("align").get()));

#ifdef __DEBUG__			
		printf("%s (%s,%s) \n",aName.c_str(),aSize.c_str(),aAlign.c_str());
#endif

		aField = field::FundamentalFactory::getInstance().getField(aName, atoi(aAlign.c_str()), this);
				
		if (aField) {
			_fundamentals[aName] = aField;
		}

		return aField;
	 
	}

	field::field* dictionary::decodeTypeDef(DOMDocument* iDocument, DOMElement* iField, 
								      std::vector<std::string>& iRecursive, int& iHasRecursive){

		std::string aType; //name
		str(aType, iField->getAttribute(str("type").get()));
	
		std::string aName;
		str(aName, iField->getAttribute(str("name").get()));

#ifdef __DEBUG__
		printf("() typedef :%s \n", aName.c_str());
#endif	

		return decodeType(iDocument, aType,  iRecursive, iHasRecursive);		

	}			

	field::field* dictionary::decodePointerType(DOMDocument* iDocument, DOMElement* iField, 
								          std::vector<std::string>& iRecursive, int& iHasRecursive){

		std::string aType; //name
		str(aType, iField->getAttribute(str("type").get()));

#ifdef __DEBUG__	
		printf("() pointer \n");
#endif

		field::field* aInnerField = decodeType(iDocument, aType, iRecursive, iHasRecursive);
		
		field::pointer_field* aReturn = new field::pointer_field("",this);
		aReturn->setField(aInnerField);

		return aReturn;	
		
	}

}
