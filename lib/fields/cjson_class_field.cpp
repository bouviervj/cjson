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
#include "cjson_class_field.h"

#include <stdio.h>
#include <iterator> 
#include <typeinfo>

using namespace jsonxx;

namespace cjson {

namespace field {

	class_field::~class_field(){

		
		for (int i=0; i<_inherited.size();i++){
			delete _inherited[i];
		}
		

		std::map<std::string, attached_field* >::iterator iter;
		for (iter = _list.begin(); iter != _list.end(); ++iter) {
			delete iter->second;
		}	

	};

	void class_field::dumpVTable(bool iHideAddress){
		
				std::stringstream ass;
				dumpVTable(ass,iHideAddress);
				std::cout << ass.str();

	}

	void class_field::dumpVTable(std::stringstream& iss, bool iHideAddress){

		iss << "Entry point table:\n";
		for (std::map<std::string,void*>::iterator it=_vtableEntryPoint.begin(); it!=_vtableEntryPoint.end(); ++it){

			if (iHideAddress) {
				iss << it->first << " => " << "0x****************" << "\n";
			} else {
				iss << it->first << " => " << std::hex << std::showbase << std::right << std::setw(18) << it->second << '\n';
			}

		}

		iss << "VTable Layout:\n";
		for (int i=0; i<_vtablelayout.size(); i++) {
			
			char buffer_name[2048]; 
			demangle(buffer_name, 2048,  _vtablelayout[i]._name.c_str());
			char buffer[2048]; 
			demangleFunctionPointer(buffer, 2048, (void *) *((long*) _vtablelayout[i]._address));
			char buffer_vt[2048]; 
			demangleFunctionPointer(buffer_vt, 2048, (void *) _vtablelayout[i]._address);
			
			if (iHideAddress) {
				iss << "[" << "0x****************" << "] ";
			} else {
				iss << "[" << std::hex << std::showbase << std::right << std::setw(18) << _vtablelayout[i]._address << "] ";
			}
			iss << buffer_vt << " ";
			if (iHideAddress) {
				iss << "0x********" << " :";
			} else {	
				iss <<  std::hex << std::right << std::setw(10) << (unsigned int) *((long*) _vtablelayout[i]._address) << " :";
			}					
			iss << " " << std::setw(20) << _vtablelayout[i]._class.c_str() << " ";
			iss << std::setw(30) << buffer_name << " expected ";
			iss << buffer << "\n";

			/*
			printf("%p %s %8x : %20s %30s expected %s\n",_vtablelayout[i]._address,
												buffer_vt
											 ,(unsigned int) *((long*) _vtablelayout[i]._address)
											 ,_vtablelayout[i]._class.c_str()
											 , buffer_name
											 , buffer);
			*/
		}

	}


	void class_field::toJson(std::ostringstream& iStream, const void* iEntryPoint){

	   std::map<std::string, attached_field* >::iterator iter;
        
#ifdef __DEBUG__
	   printf("###########\n");	
	   printf("Class %s \n", getName().c_str());
	   for (iter = _list.begin(); iter != _list.end(); ++iter) {
			printf("Field %s %p\n", iter->first.c_str(),(void*) iter->second->getField());
	   }
	   printf("###########\n");	
#endif



	   iStream << "{";
        for (iter = _list.begin(); iter != _list.end(); ++iter) {
	
			attached_field* aField = iter->second;
			if (aField) {
#ifdef __DEBUG__
				printf("Field %s \n", iter->first.c_str());
				printf("Offset (%d) \n",(int) aField->getOffset());
				if (aField->getField()) {
					printf("%s\n", aField->getField()->getName().c_str());
					printf("Type: %s\n", typeid(aField->getField()).name());
				} else {
					printf("no field defined.\n");				
				}
#endif
				iStream << "\"" << iter->first << "\":";
				void* aFieldAddress = (void*) ((unsigned long) iEntryPoint+aField->getOffset()/8); 				
				aField->getField()->toJson(iStream, aFieldAddress);
#ifdef __DEBUG__
				printf("distance:%d\n",(int) std::distance(iter,_list.end()));
#endif
				if (std::distance(iter,_list.end()) > 1) iStream << ",";
			}

	    }

	    iStream << "}";	

	}

	void* class_field::fromJson(const std::string& iJson){
		void* aMem = malloc(_size/8);
#ifdef __DEBUG__
		printf("Allocate class size %d\n",_size/8);
#endif
		
		jsonxx::Value aValue;
	     aValue.parse(iJson);

		fromJson(aValue, aMem);

		return aMem;
	}


	void class_field::consolidateVTablePointers(void* iEntryPoint, std::map<std::string, void* >& iEntryPoints) {

		// Prepare Vtable pointers
		std::map<std::string, void* >::iterator it;
		std::map<std::string,long>::iterator it_off;
		for (it = iEntryPoints.begin(); it != iEntryPoints.end(); ++it) {
			
			it_off = getVTableOffsets().find(it->first);			
			if (it_off!=getVTableOffsets().end()) {

				long aOffset = it_off->second;
#ifdef __DEBUG__
				printf("offset %s %p\n",it->first.c_str(), (void*) aOffset);
#endif
				*((long*) ((long) iEntryPoint+aOffset/8)) =(long) it->second;
	
			}	

		}

		for (int i=0;i<_inherited.size(); i++) {
			long aOffset = _inherited[i]->getOffset();
#ifdef __DEBUG__
			printf("consolidate on %s \n", _inherited[i]->getField()->getName().c_str());
#endif
			class_field* aClass = dynamic_cast<class_field*>(_inherited[i]->getField());
			aClass->consolidateVTablePointers((void*) ((long) iEntryPoint+aOffset/8),iEntryPoints); 
		}

	}

	void class_field::fromJson(const jsonxx::Value& iJson, void* iEntryPoint){

		// Set class members values
		std::map<std::string, attached_field* >::iterator iter;
	     for (iter = _list.begin(); iter != _list.end(); ++iter) {
			attached_field* aField = iter->second;
			if (aField) {
				Value aValue = iJson.get<Object>().get<Value>(iter->first);	
				void* aFieldAddress = (void*) ((unsigned long) iEntryPoint+aField->getOffset()/8);		
				aField->getField()->fromJson(aValue, aFieldAddress);
			}
	     }
		
		consolidateVTablePointers(iEntryPoint, _vtableEntryPoint);

	}

	void class_field::toCpp(std::ostringstream& iStream){

		std::map<std::string, attached_field* >::iterator iter;
          iStream << "class " << _name << " ";
		iStream << "{\n";
		for (iter = _list.begin(); iter != _list.end(); ++iter) {
			attached_field* aField = iter->second;
			if (aField) {
				aField->getField()->toCpp(iStream);
				iStream << " " << iter->first << "(" << aField->getOffset() << ")" <<";\n"; 								
			}

	     }
	     iStream << "}\n";	

	}


}

}
