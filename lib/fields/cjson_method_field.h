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
#ifndef _CJSON_METHOD_FIELD_H_
#define _CJSON_METHOD_FIELD_H_

#include "cjson_field.h"
#include <map>
#include "cjson_abi_struct.h"

namespace cjson {

namespace field {

	class method_field : public field {

		private :

			ABI_MElem _mElem;
			
		public :

			method_field(const std::string& iMangled, const std::string& iName, cjson::dictionary* iDict): field(iName,iDict){
				_mElem._mangled = iMangled;
				_mElem._name = iName;
			};
			virtual ~method_field(){};

			const ABI_MElem& getABI(){
				return _mElem;	
			}

			ABI_MElem& getABIInst(){
				return _mElem;	
			}

			void dumpABI(){
				_mElem.dump();
			}

			void addArgument(const std::string& iName){
				_mElem._arguments.push_back(iName);
			}

			virtual void toJson(std::ostringstream& iStream, const void* iEntryPoint);
			virtual void* fromJson(const std::string& iJson);	
			virtual void fromJson(const jsonxx::Value& iJson, void* iEntryPoint);
			
			virtual void toCpp(std::ostringstream& iStream);		

	};


}}


#endif
