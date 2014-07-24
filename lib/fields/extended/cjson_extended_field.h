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
#ifndef _CJSON_EXTENDED_FIELD_H_
#define _CJSON_EXTENDED_FIELD_H_

#include <stdio.h>
#include <map>
#include "../cjson_field.h"
#include "../cjson_class_field.h"
#include <utils.h>

namespace cjson {
	
	class regexp;

namespace field {

	class extended_field : public field {

		protected:

			class_field* _repField;

		public:

			extended_field(const std::string& iName, cjson::dictionary* iDict) : field(iName,iDict){	
			}
			virtual ~extended_field(){		
			};

			virtual void toJson(std::ostringstream& iStream, const void* iEntryPoint)=0;
			virtual void* fromJson(const std::string& iJson) = 0;
			virtual void fromJson(const jsonxx::Value& iJson, void* iEntryPoint)=0;

			void setRepField(class_field* iRepField){
				_repField = iRepField;
			}

			class_field* getRepField(){
				return _repField;
			}

			virtual int getSize(){
				return _repField->getSize();
			}

	};

	class FieldTypeFactory {

		public :

			virtual extended_field* getField(cjson::dictionary* iDict) = 0;
	
	};

	class ExtendedFieldFactory {

		public :		

			static ExtendedFieldFactory& getInstance();

			bool matches(int iIndex, const std::string& iStr);

			void storeFactory(const std::string& iFieldType, FieldTypeFactory* iFactory);

			extended_field* getField(std::string& iFieldType, cjson::dictionary* iDict) {

        			for (int i=0;i <_factories.size(); i++) {
	
					if (matches(i, iFieldType)) {
						return _factories[i].second->getField(iDict);
					}

				}

				return NULL;
			}

			void addFactory(const std::string& iFieldType, FieldTypeFactory* iFactory){
#ifdef __DEBUG__
				printf("Registering field factory: %s for [%p] from [%p]\n",iFieldType.c_str(),this,iFactory);
#endif
				storeFactory(iFieldType ,iFactory);
			}
	
		private : 		
	
			std::vector<std::pair<regexp* ,FieldTypeFactory*> > _factories;

		public :
			static ExtendedFieldFactory _instance;

	};

	template <class FIELD> 
	class ExtendedFieldTypeFactory : FieldTypeFactory {

		private :			

			std::string _name;

		public :	

			ExtendedFieldTypeFactory(const char* iFieldTypeName){				
				std::string aName(iFieldTypeName);
				_name = aName;
				ExtendedFieldFactory::getInstance().addFactory(aName, this);
			}

			ExtendedFieldTypeFactory<FIELD>& getInstance();

			extended_field* getField(cjson::dictionary* iDict) {
				return new FIELD(_name, iDict);
			}

	};

	#define ADD_FIELD_TYPE_FACTORY(FIELDCLASS) static const cjson::field::ExtendedFieldTypeFactory<FIELDCLASS>* _instance;
	#define CREATE_FIELD_TYPE_FACTORY(TYPENAME,FIELDCLASS) const cjson::field::ExtendedFieldTypeFactory<FIELDCLASS>* FIELDCLASS::_instance = new cjson::field::ExtendedFieldTypeFactory<FIELDCLASS>(TYPENAME); 

}}

#endif
