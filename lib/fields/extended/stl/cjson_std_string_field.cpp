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
#include "cjson_std_string_field.h"

#include "../cjson_dictionary.h"

using namespace jsonxx;
  
  // FROM STL
  /*
   *  A string looks like this:
   *
   *  @code
   *                                        [_Rep]
   *                                        _M_length
   *   [basic_string<char_type>]            _M_capacity
   *   _M_dataplus                          _M_refcount
   *   _M_p ---------------->               unnamed array of char_type
   *  @endcode
   *
   *  Where the _M_p points to the first character in the string, and
   *  you cast it to a pointer-to-_Rep and subtract 1 to get a
   *  pointer to the header.
   *
   *  This approach has the enormous advantage that a string object
   *  requires only one allocation.  All the ugliness is confined
   *  within a single %pair of inline functions, which each compile to
   *  a single @a add instruction: _Rep::_M_data(), and
   *  string::_M_rep(); and the allocation function which gets a
   *  block of raw bytes and with room enough and constructs a _Rep
   *  object at the front.
   *
   *  The reason you want _M_data pointing to the character %array and
   *  not the _Rep is so that the debugger can see the string
   *  contents. (Probably we should add a non-inline member to get
   *  the _Rep for the debugger to use, so users can check the actual
   *  string length.)
   */

namespace cjson {

namespace field {

	CREATE_FIELD_TYPE_FACTORY("^std::string$",std_string_field)

	void std_string_field::toJson(std::ostringstream& iStream, const void* iEntryPoint){

		std::string* aStr = (std::string*) iEntryPoint;
#ifdef __DEBUG__
		printf("%s\n",aStr->c_str());
#endif
		iStream << "\"" << *aStr << "\"";

	}
	
	void std_string_field::toJsonMap(std::ostringstream& iStream, const void* iEntryPoint){
		toJson(iStream, iEntryPoint);
	}
	
	void* std_string_field::fromJson(const std::string& iJson){
	// decodes a single char
	// TODO to implement
	}

	

	//typedef std::string::_Rep* (std::string::*METHOD) (void);

	void std_string_field::fromJson(const jsonxx::Value& iJson, void* iEntryPoint){
		std::string aString = iJson.get<String>();
#ifdef __DEBUG__
		printf("Relocated string %s\n",aString.c_str());
#endif

		cjson::field::field* aDataStruct = _dictionary->getDataStruct("std::string::_Rep");
#ifdef __DEBUG__
		printf("_Rep allocation size:%d\n",aDataStruct->getSize());
#endif
		
		// Here using new 
		void* aAllocated = new char[aDataStruct->getSize()/8+sizeof(char)];

		void* aStringAddress = (void*) ((std::string*) iEntryPoint);

		*((long*) aStringAddress) = ((long) aAllocated+aDataStruct->getSize()/8);

		*((std::string*) aStringAddress) = aString; // TODO more work according to stl	lmake
#ifdef __DEBUG__
		printf("%s\n", ((std::string*) aStringAddress)->c_str());
#endif
	}

	bool std_string_field::less(const jsonxx::Value& iJsonL, const jsonxx::Value& iJsonR){

		std::string aStringL = iJsonL.get<String>();
		std::string aStringR = iJsonR.get<String>();

		return aStringL < aStringR;		
	
	}

	void std_string_field::fromJsonMap(const jsonxx::Value& iJson, void* iEntryPoint){
		fromJson(iJson,iEntryPoint);
	}

	void std_string_field::toCpp(std::ostringstream& iStream){
		iStream << "std::string" ;
	}


}}
