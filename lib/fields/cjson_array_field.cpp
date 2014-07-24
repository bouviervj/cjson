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
#include "cjson_array_field.h"

#include <stdio.h>

using namespace jsonxx;

namespace cjson {

namespace field {

	void array_field::toJson(std::ostringstream& iStream, const void* iEntryPoint){

#ifdef __DEBUG__
	   printf("Entry Point [%p]\n",iEntryPoint);	
#endif
	   int step = _size/_length;
#ifdef __DEBUG__
	   printf("Step %d\n",step);		
	   //printf("Offset %d\n",_offset);	
#endif
	   iStream << "[";
	   void* address = (void*) ((unsigned long) iEntryPoint);
        for (int i=0; i<_length-1; i++) {

#ifdef __DEBUG__
			printf("Field Size: %d\n",_innerField->getSize());
#endif			
			_innerField->toJson(iStream, address);
			address= (void*) ((long) address +step/8); // This works if not packed
			iStream << ",";
	   }
	   _innerField->toJson(iStream, address);
	   iStream << "]";

	}

	void* array_field::fromJson(const std::string& iJson){
		// decodes a single char
		// TODO to implement
	}


	void array_field::fromJson(const jsonxx::Value& iJson, void* iEntryPoint){
		
		int step = _size/_length;
		void* address = (void*) ((long)iEntryPoint);
		for (int i=0; i<_length; i++) {	
			Value aJsonValue = iJson.get<Array>().get<Value>(i);	
			_innerField->fromJson(aJsonValue, address);
			address= (void*) ((long) address +step/8); // This works if not packed
	     }
	}

	void array_field::toCpp(std::ostringstream& iStream){

		_innerField->toCpp(iStream);
		iStream << "[" << _length << "]" ;

	}


}

}
