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
#ifndef _CJSON_RECURSIVE_FIELD_H_
#define _CJSON_RECURSIVE_FIELD_H_

#include "cjson_field.h"
#include <vector>

namespace cjson {

namespace field {

	class recursive_field : public field {

		private :

			int _align;

		public :

			recursive_field(const std::string& iName, cjson::dictionary* iDict): field(iName,iDict){
				
			};
			virtual ~recursive_field(){};

			virtual void toJson(std::ostringstream& iStream, const void* iEntryPoint);
			virtual void toJsonMap(std::ostringstream& iStream, const void* iEntryPoint);
			virtual void* fromJson(const std::string& iJson);
			virtual void fromJson(const jsonxx::Value& iJson, void* iEntryPoint);
			virtual void fromJsonMap(const jsonxx::Value& iJson, void* iEntryPoint);	

			virtual void toCpp(std::ostringstream& iStream);

	};


}}


#endif
