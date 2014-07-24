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
#include "cjson_recursive_field.h"

#include <stdio.h>

using namespace jsonxx;

namespace cjson {

namespace field {

void recursive_field::toJson(std::ostringstream& iStream, const void* iEntryPoint){
		
	char* aData = (char*) &(iEntryPoint); // TODO ?? WTF

}

void recursive_field::toJsonMap(std::ostringstream& iStream, const void* iEntryPoint){
	abort(); // to implement
}

void* recursive_field::fromJson(const std::string& iJson){
	// decodes a single char
	// TODO to implement
}


void recursive_field::fromJson(const jsonxx::Value& iJson, void* iEntryPoint){


}

void recursive_field::fromJsonMap(const jsonxx::Value& iJson, void* iEntryPoint){
	abort(); // not coded yet you have to get string value then convert it to a number
}

void recursive_field::toCpp(std::ostringstream& iStream){
	iStream << "(recursive)" << _name;
}


}
}
