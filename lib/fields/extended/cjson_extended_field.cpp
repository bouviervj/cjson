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


#include "cjson_extended_field.h"
#include "cjson_regexp.h"

namespace cjson {

namespace field {

		ExtendedFieldFactory& ExtendedFieldFactory::getInstance(){
			static ExtendedFieldFactory _instance;		
			return _instance;
		}

		bool ExtendedFieldFactory::matches(int iIndex, const std::string& iStr){
			return _factories[iIndex].first->matches(iStr);
		}

		void ExtendedFieldFactory::storeFactory(const std::string& iRegExp, FieldTypeFactory* iFactory){

			regexp* aRegExp = new regexp( iRegExp );
			_factories.push_back(std::pair<regexp*,FieldTypeFactory*>( aRegExp , iFactory));

		}


}}



