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
#include "cjson_pointer_field.h"

#include <stdio.h>

using namespace jsonxx;

namespace cjson {

namespace field {

	void pointer_field::toJson(std::ostringstream& iStream, const void* iEntryPoint){

		void** aData = (void**) (iEntryPoint);
		if (*aData==0) {
			iStream <<"null";
			return;
		}
#ifdef __DEBUG__
		printf("pointer address %p\n",*aData);
		printf("sub element size %d\n",(int)  _innerField->getSize());
#endif
		long aSize = *((long*) ((long) (*aData)-8));

#ifdef __DEBUG__
		printf("pointer size %d\n",(int) aSize);
#endif

		long aNumberOfElements =  aSize/(_innerField->getSize()/8);
#ifdef __DEBUG__
		printf("number of elements %d\n", (int) aNumberOfElements);	
#endif

		void* aAddress = *aData;
		iStream << "[";
		for (int i=0; i<aNumberOfElements-1; i++) {
			_innerField->toJson(iStream, aAddress);
			iStream << ",";
			aAddress = (void*) ((long)aAddress+_innerField->getSize()/8);
		}
		_innerField->toJson(iStream, aAddress);
		iStream << "]";	

	}

	void* pointer_field::fromJson(const std::string& iJson){
		// decodes a single char
		// TODO to implement
	}

	void pointer_field::fromJson(const jsonxx::Value& iJson, void* iEntryPoint){

		Array aArray =  iJson.get<Array>();
		int aArraySize = aArray.size();

		int aFieldSize = _innerField->getSize()/8;
		int aTotalSize = aFieldSize*aArraySize;

		void** aData = (void**) (iEntryPoint);
		*aData = malloc(aTotalSize);

		void* aAddress = *aData;
		for (int i=0; i<aArraySize; i++) {
			_innerField->fromJson(aArray.get<Value>(i), aAddress);
			aAddress = (void*) ((long)aAddress + aFieldSize);
		}
		
	}	

	void pointer_field::toCpp(std::ostringstream& iStream){

		_innerField->toCpp(iStream);
		iStream << "*" ;

	}

}

}
