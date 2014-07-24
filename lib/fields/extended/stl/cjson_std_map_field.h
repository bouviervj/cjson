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
#ifndef _CJSON_STD_MAP_FIELD_H_
#define _CJSON_STD_MAP_FIELD_H_

#include <string>
#include "../cjson_extended_field.h"

namespace cjson {

namespace field {


	class std_map_field : public extended_field {

		public:

			std_map_field(const std::string& iName, cjson::dictionary* iDict) : extended_field(iName,iDict) {
				_init = false;	
			}
			virtual ~std_map_field(){};		

			unsigned long getAlignedSize(unsigned long iSize);

			void decodeKeyValuePair(std::ostringstream& iStream, std::_Rb_tree_node_base* aNode, field* iFirst, field* iSecond);
			void readRedBlackTree(std::vector<std::string>& iElementsList , std::_Rb_tree_node_base* iNode, field* iFirst, field* iSecond);

			void copyNode(std::_Rb_tree_node<std::pair<field_value, jsonxx::Value> >* iNodeToCopy, field* iSecond, void* iParent, void*& ioPointer, void*& iFirst, void*& iLast);

			virtual void toJson(std::ostringstream& iStream, const void* iEntryPoint);
			virtual void toJsonMap(std::ostringstream& iStream, const void* iEntryPoint);
			virtual void* fromJson(const std::string& iJson);
			virtual void fromJson(const jsonxx::Value& iJson, void* iEntryPoint);
			virtual void fromJsonMap(const jsonxx::Value& iJson, void* iEntryPoint);

			virtual void toCpp(std::ostringstream& iStream);

		private:

			bool _init;
			field* _first;
			field* _second;

			void init();

		public :

		ADD_FIELD_TYPE_FACTORY(cjson::field::std_map_field)	

	};

	

}}



#endif
