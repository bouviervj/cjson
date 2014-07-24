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
#ifndef _CJSON_CLASS_FIELD_H_
#define _CJSON_CLASS_FIELD_H_

#include "cjson_field.h"
#include <iomanip>
#include <map>
#include "cjson_abi_struct.h"
#include "./utils/demangle.h"

namespace cjson {

namespace field {

	class class_field : public field {

		private :

			std::map<std::string , attached_field* > _list;
			ABI_Inh _inh;

			std::vector<attached_field*> _inherited; // inherited classes are attached fields
											 // we get offset to relocate their fields
			
			std::vector<cjson::ABI_Layout_Element> _vtablelayout;
			std::map<std::string,void*>  _vtableEntryPoint;
			std::map<std::string,long>  _vtableOffsets;

		public :

			class_field(const std::string& iName, cjson::dictionary* iDict): field(iName,iDict){
			};
			virtual ~class_field();

			const ABI_Inh& getABI(){
				return _inh;	
			}

			ABI_Inh& getABIInst(){
				return _inh;	
			}

			void dumpABI(){
				_inh.dump();
			}

			void addInherited(const unsigned long iOffset, class_field* aClassField){
				_inherited.push_back(new attached_field(iOffset,aClassField) );
			}

			std::vector<cjson::ABI_Layout_Element>& getVTableLayout(){
				return _vtablelayout;
			}

			std::map<std::string,void*>& getVTableEntryPoint(){
				return _vtableEntryPoint;
			}
		
			std::map<std::string,long>& getVTableOffsets(){
				return _vtableOffsets;
			}

			void dumpVTable(bool iHideAddress = false);

			void dumpVTable(std::stringstream& iss, bool iHideAddress = false);
			

			void addField(const std::string& iName,const int iOffset, field* iField){
				_list[iName] = new attached_field( iOffset, iField);
			}

			std::map<std::string , attached_field* >& getList(){
				return _list;
			}

			int getListSize(){
				return _list.size();
			}

			attached_field* innerField(const std::string& iName){
				return _list[iName];
			}

			void consolidateVTablePointers(void* iEntryPoint,std::map<std::string, void* >& iEntryPoints);

			virtual void toJson(std::ostringstream& iStream, const void* iEntryPoint);
			virtual void* fromJson(const std::string& iJson);	
			virtual void fromJson(const jsonxx::Value& iJson, void* iEntryPoint);
			
			virtual void toCpp(std::ostringstream& iStream);

	};


}}


#endif
