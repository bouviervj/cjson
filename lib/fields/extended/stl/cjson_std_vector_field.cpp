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
#include "cjson_std_vector_field.h"

#include "../cjson_dictionary.h"

using namespace jsonxx;
  
namespace cjson {

namespace field {

	#define REG_EXP_STD_VECTOR "^std::vector<(.*),\\s(.*)\\s>$"

	CREATE_FIELD_TYPE_FACTORY( REG_EXP_STD_VECTOR ,std_vector_field)

	void std_vector_field::toJson(std::ostringstream& iStream, const void* iEntryPoint){

		void* aAddress = (void*) (iEntryPoint);
		void* aStart = (void*) *((long*) aAddress);
		void* aEnd = (void*) *((long*) ((long)aAddress+ENV_SIZE_BYTES));
#ifdef __DEBUG__
		printf("Vector start: %p\n",aStart);
		printf("Vector end: %p\n",aEnd);
		std::map<std::string ,attached_field* > aList = _repField->getList();
		std::map<std::string, attached_field* >::iterator iter;	
			
		for (iter = aList.begin(); iter != aList.end(); ++iter) {	
			printf("field : %s\n", iter->first.c_str());
		}

#endif

		attached_field* aAttField = dynamic_cast<attached_field*>(_repField->getList().begin()->second); // _M_impl(0)
		class_field* aField = dynamic_cast<class_field*>(aAttField->getField());

		int iFieldSize = aField->getListSize();
#ifdef __DEBUG__
		printf("Number of fields : %d\n",iFieldSize); // Should be 3
#endif

		attached_field* aPAttField = dynamic_cast<attached_field*>(aField->innerField("_M_start")); // Here I can have the offset should be 0; and at list a pointer
		pointer_field* aPField = dynamic_cast<pointer_field*>(aPAttField->getField());
		field* aFField = aPField->innerField();
#ifdef __DEBUG__		
		printf("Data Type:%s (%d)\n",aFField->getName().c_str(),aFField->getSize());
#endif

		int step = aFField->getSize()/8;
		int length = ((long) aEnd- (long)aStart)/step;
#ifdef __DEBUG__
		printf("Vector step: %d\n",step);
		printf("Vector size: %d\n",length);
#endif

		iStream << "[";
		for (int i=0; i<length; i++) {
			void* aFieldAddress = (void*) ((long) aStart + i*step);	
			aFField->toJson(iStream, aFieldAddress);
			if (i<length-1) iStream << ",";
		}
		iStream << "]";
	}
	
	void* std_vector_field::fromJson(const std::string& iJson){
	// decodes a single char
	// TODO to implement
	}

	void std_vector_field::fromJson(const jsonxx::Value& iJson, void* iEntryPoint){

		Array aArray =  iJson.get<Array>();
		int aArraySize = aArray.size();

		attached_field* aAttField = _repField->getList().begin()->second;
		class_field* aField = dynamic_cast<class_field*>(aAttField->getField()); // _M_impl(0)
		int iFieldSize = aField->getListSize();
#ifdef __DEBUG__
		printf("Number of fields : %d\n",iFieldSize); // Should be 3
#endif

		attached_field* aPAttField = aField->innerField("_M_start");
		pointer_field* aPField = dynamic_cast<pointer_field*>(aPAttField->getField()); // Here I can have the offset should be 0; and at list a pointer
		field* aFField = aPField->innerField();	
#ifdef __DEBUG__	
		printf("Data Type:%s (%d)\n",aFField->getName().c_str(),aFField->getSize());
#endif

		int aFieldSize = aFField->getSize()/8;
		int aTotalSize = aFieldSize*aArraySize;

		void** aData = (void**) (iEntryPoint);
		*aData = malloc(aTotalSize);

		void* aAddress = *aData;
		for (int i=0; i<aArraySize; i++) {
			aFField->fromJson(aArray.get<Value>(i), aAddress);
			aAddress = (void*) ((long)aAddress + aFieldSize);
		}
		
		*((void**) ((long) aData+ENV_SIZE_BYTES)) = aAddress; // _M_finish
		*((void**) ((long) aData+ENV_SIZE_BYTES*2)) = aAddress;// _M_end_of_storage
		// Then setup the 3 fields

	}

	void std_vector_field::toCpp(std::ostringstream& iStream){
		iStream << _name.c_str() << "[" ;
		_repField->toCpp(iStream);
		iStream << "] ";
	}


}}
