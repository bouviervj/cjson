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
#include "cjson_fundamental_field.h"

namespace cjson {

namespace field {

		FundamentalFactory::FundamentalFactory(){

		}

		FundamentalFactory& FundamentalFactory::getInstance(){
			static FundamentalFactory _instance;		
			return _instance;
		} 

		CREATE_FUNDAMENTAL_TYPE_FACTORY(bool, 		 	 BOOL     )
		CREATE_FUNDAMENTAL_TYPE_FACTORY(char, 		 	 CHAR     )
		CREATE_FUNDAMENTAL_TYPE_FACTORY(unsigned char,	 UCHAR    )
		CREATE_FUNDAMENTAL_TYPE_FACTORY(int, 		 	 INT      )
		CREATE_FUNDAMENTAL_TYPE_FACTORY(unsigned int, 	 UINT     )
		CREATE_FUNDAMENTAL_TYPE_FACTORY(long, 		 	 LONG     )
		CREATE_FUNDAMENTAL_TYPE_FACTORY(unsigned long,	 ULONG    )
		CREATE_FUNDAMENTAL_TYPE_FACTORY(long long,	 	 LONGLONG )
		CREATE_FUNDAMENTAL_TYPE_FACTORY(unsigned long long,ULONGLONG)
		CREATE_FUNDAMENTAL_TYPE_FACTORY(float, 		 	 FLOAT    )
		CREATE_FUNDAMENTAL_TYPE_FACTORY(double, 	      DOUBLE   )
		
		CREATE_NAMED_FUNDAMENTAL_TYPE_FACTORY(long long unsigned int,"long long unsigned int",LONGLONGUINT)
		CREATE_NAMED_FUNDAMENTAL_TYPE_FACTORY(long unsigned int,"long unsigned int",LONGUINT)
		CREATE_NAMED_FUNDAMENTAL_TYPE_FACTORY(short unsigned int,"short unsigned int",SHORTUINT)
		CREATE_NAMED_FUNDAMENTAL_TYPE_FACTORY(long int,"long int",LONGINT)
		CREATE_NAMED_FUNDAMENTAL_TYPE_FACTORY(signed char,"signed char",SIGNEDCHAR)

		template<>
		void fundamental_field<char>::toJson(std::ostringstream& iStream, const void* iEntryPoint){
			char* aData = AD(char,iEntryPoint);
			iStream << (int) *aData;
		}

		template<>
		void fundamental_field<float>::toJson(std::ostringstream& iStream, const void* iEntryPoint){
			float* aData = AD(float,iEntryPoint);
			iStream << std::fixed << (float) *aData;
		}

		field* FundamentalFactory::getField(const std::string& iFieldType, int iAlign, cjson::dictionary* iDict) {						     
			GenericFundamentalFieldFactory* aFact = _factories[iFieldType];
			if (aFact) {
				return aFact->getField(iAlign,iDict);
			}
#ifdef __DEBUG__
			printf("missing requested type:%s\n",iFieldType.c_str());	
#endif
			return NULL;
		}

		void FundamentalFactory::addFactory(const std::string& iFieldType, GenericFundamentalFieldFactory* iFactory){
#ifdef __DEBUG__
			printf("Registering fundamental factory: %s for [%p] from [%p]\n",iFieldType.c_str(),this,iFactory);
#endif
			if (_factories.find(iFieldType)!=_factories.end()) {
				abort();
			}

			_factories[iFieldType] = iFactory;
		}
		
		

}}

