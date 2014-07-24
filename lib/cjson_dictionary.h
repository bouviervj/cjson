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
#ifndef _CJSON_DICTIONARY_H_
#define _CJSON_DICTIONARY_H_

#include <string>
#include <map>
#include <xercesc/parsers/XercesDOMParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>

#include <typeinfo>
#include <cjson_smart_pointer.h>

// Fields declaration
#include <fields/cjson_field.h>
#include <fields/cjson_class_field.h>
#include <fields/cjson_pointer_field.h>

// Reading symbols
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <link.h>
#include <stdlib.h>
#include <string>
#include <vector>

using namespace xercesc;

// Malloc redef
long pointer_size(void *ptr);

namespace cjson {

	class xmlErrorHandler : public HandlerBase
	{
	  public:
	    void fatalError(const SAXParseException &exc);
	};

	class dictionary {

		// main functions for the dictionary
		public :

			dictionary(std::string iFilename);
			~dictionary();

			template<class T>
			std::string toJson(T* aPointerToObject);

			template<class T>
			T* fromJson(const std::string& iJson);


		// Inner functionnalities
		public :
		
			// To retrieve structures
			field::field* getDataStruct(const std::string& iName) {
				field::field* aField;
				aField = _resources[iName];
				if (!aField) {
					aField = _fundamentals[iName];
				}
				return aField;
			}

			// Release 
			static void releaseFunction(XMLCh* p);
			static void str(std::string& ioString, const XMLCh* aName);
			static cjson::smart_ptr<XMLCh> str(const std::string& iStr);

			cjson::smart_ptr<XMLCh> operator=(const std::string& rhs);

			void* getVTableAddress(std::string iClassName);

		private :

			void init();
			void recordStructures(DOMDocument* iDocument);
			field::field* decodeClassOrStruct(DOMDocument* iDocument, DOMElement* iClass,  
								   std::vector<std::string>& iRecursive, int& iHasRecursive);
			
			field::field* decodeMember(DOMDocument* iDocument, const std::string& iID, unsigned long& ioOffset, std::string& ioMemberName, int& iMethod, 
								   bool& ioVirtual, 
								   std::vector<std::string>& iRecursive, int& iHasRecursive);
			field::field* decodeConstructor(DOMDocument* iDocument, DOMElement* iField , std::string& ioMethodName);
			field::field* decodeDestructor(DOMDocument* iDocument, DOMElement* iField , std::string& ioMethodName, bool& ioVirtual); 
			field::field* decodeMethod(DOMDocument* iDocument, DOMElement* iField , std::string& ioMethodName, bool& ioVirtual);
			field::field* decodeField(DOMDocument* iDocument, DOMElement* iClass, unsigned long& ioOffset, std::string& ioMemberName, 
								 std::vector<std::string>& iRecursive, int& iHasRecursive); 

			field::field* decodeType(DOMDocument* iDocument, const std::string& iID,  
								 std::vector<std::string>& iRecursive, int& iHasRecursive);
			field::field* decodeEnumeration(DOMDocument* iDocument, DOMElement* iField);
			field::field* decodeFundamentalType(DOMDocument* iDocument, DOMElement* iField);
			field::field* decodeTypeDef(DOMDocument* iDocument, DOMElement* iField, 
								    std::vector<std::string>& iRecursive, int& iHasRecursive);
			field::field* decodePointerType(DOMDocument* iDocument, DOMElement* iField, 
								   std::vector<std::string>& iRecursive, int& iHasRecursive);	
			field::field* decodeArrayType(DOMDocument* iDocument, DOMElement* iField ,
								   std::vector<std::string>& iRecursive, int& iHasRecursive);

			bool haveRecursion(std::vector<std::string>& iRecursive, std::string& iName);

			void removeAttr(const std::string iID, std::string& ioShorten);		

			std::string _filename;

			XercesDOMParser* m_parser;
    			xmlErrorHandler* m_errHandler;

			DOMDocument* _document;

			std::map<std::string, field::field*> _resources;
			std::map<std::string, field::field*> _fundamentals;
			std::map<std::string,void*> _symbolVTableAddresses;

			// Symbol reading 
			static bool startsWith(const char *pre, const char *str);
			static int retrieve_symbolnames(struct dl_phdr_info* info, size_t info_size, void* symbol_names_map);

			
	};

	template<class T>
	std::string dictionary::toJson(T* aPointerToObject) {	
	
		std::string aClassName = typeid(*aPointerToObject).name();	
	
		char aBuffer[2048];
		demangle(aBuffer, 2048, aClassName.c_str());
		cjson::field::field* aDataStruct = getDataStruct(std::string(aBuffer));

		if (aDataStruct) { // Here we can decode the object
			std::ostringstream aStream;
			aDataStruct->toJson(aStream, aPointerToObject);
			return aStream.str();
		}

		return "";

	}
	
	template<class T>
	T* dictionary::fromJson(const std::string& iJson){

		T* aObject;
		std::string aClassName = typeid(aObject).name();	
	
		char aBuffer[2048];
		demangle(aBuffer, 2048, aClassName.c_str());
		
		aBuffer[strlen(aBuffer)-1] = 0;
	
		cjson::field::field* aDataStruct = getDataStruct(std::string(aBuffer));

		if (aDataStruct) {
			return reinterpret_cast<T*> (aDataStruct->fromJson(iJson));
		}

		return 0;

	}


}

#endif
