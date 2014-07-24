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
#ifndef _CJSON_FUNDAMENTAL_FIELD_H_
#define _CJSON_FUNDAMENTAL_FIELD_H_

#include "cjson_field.h"
#include <vector>
#include <stdio.h>
#include "demangle.h"
#include <typeinfo>

using namespace jsonxx;

namespace cjson {

namespace field {

	template<typename T>
	std::string typeid_dm(){
		char buffer[2048];
		demangle(buffer, 2048, typeid(T).name());
		return std::string(buffer);
	}

	template<typename T>
	class fundamental_field : public field {

		private :

			int _align;

		public :

			fundamental_field(int iAlign, cjson::dictionary* iDict): field(typeid_dm<T>(),iDict){
				_align = iAlign;
				_size = sizeof(T)*8; 
			};
			virtual ~fundamental_field(){};

			virtual void toJson(std::ostringstream& iStream, const void* iEntryPoint);
			virtual void toJsonMap(std::ostringstream& iStream, const void* iEntryPoint);
			virtual void* fromJson(const std::string& iJson);	
			virtual void fromJson(const jsonxx::Value& iJson, void* iEntryPoint);
			virtual void fromJsonMap(const jsonxx::Value& iJson, void* iEntryPoint);

			virtual bool less(const jsonxx::Value& iJsonL, const jsonxx::Value& iJsonR);	

			virtual void toCpp(std::ostringstream& iStream);	

	};

	template<typename T>
	void fundamental_field<T>::toJson(std::ostringstream& iStream, const void* iEntryPoint){
		T* aData = AD(T,iEntryPoint);
		iStream << *aData;
	}

	template<>
	void fundamental_field<char>::toJson(std::ostringstream& iStream, const void* iEntryPoint);

	template<>
	void fundamental_field<float>::toJson(std::ostringstream& iStream, const void* iEntryPoint);

	template<typename T>
	void fundamental_field<T>::toJsonMap(std::ostringstream& iStream, const void* iEntryPoint){
		iStream << "\"";
		toJson(iStream, iEntryPoint);
		iStream << "\"";
	}

	template<typename T>
	void* fundamental_field<T>::fromJson(const std::string& iJson){
		void* aData = malloc(sizeof(T));
		fromJson(iJson, aData);
          return aData;
	}

	template<typename T>
	void fundamental_field<T>::fromJson(const jsonxx::Value& iJson, void* iEntryPoint){
		if (iJson.empty()) abort();
		LD(T,iEntryPoint,iJson.get<float>());
	}

	template<typename T>
	void fundamental_field<T>::fromJsonMap(const jsonxx::Value& iJson, void* iEntryPoint){
		std::string aRep = iJson.get<String>();
		LD(T,iEntryPoint,atof(aRep.c_str())); // TODO change this to support direct int conversion
	}

	template<typename T>
	void fundamental_field<T>::toCpp(std::ostringstream& iStream){
		iStream << typeid_dm<T>();
	}

	template<typename T>
	bool fundamental_field<T>::less(const jsonxx::Value& iJsonL, const jsonxx::Value& iJsonR){
	
		std::string aRep = iJsonL.get<String>(); // in fact a json map decode less will be always used in map (for the moment)
		T aL = (T) atof(aRep.c_str()); // TODO change this to support direct int conversion
		aRep = iJsonR.get<String>();
		T aR = (T) atof(aRep.c_str()); // TODO change this to support direct int conversion
	
		return aL < aR;
	}

	class GenericFundamentalFieldFactory {

		public :
			virtual field* getField(int iAlign, cjson::dictionary* iDict) = 0;
		
	};

	class FundamentalFactory {

		public :		

			FundamentalFactory();

			static FundamentalFactory& getInstance();

			field* getField(const std::string& iFieldType, int iAlign, cjson::dictionary* iDict);

			void addFactory(const std::string& iFieldType, GenericFundamentalFieldFactory* iFactory);
	
		private : 		
	
			std::map<std::string,GenericFundamentalFieldFactory*> _factories;

		public :
			static FundamentalFactory _instance;

	};


	template<typename T> 
	class FundamentalFieldFactory : GenericFundamentalFieldFactory {

		public :	

			FundamentalFieldFactory(){
				char buffer[2048];
				demangle(buffer, 2048, typeid(T).name());
				FundamentalFactory::getInstance().addFactory( std::string(buffer) , this);
			}

			FundamentalFieldFactory(const std::string& iName){
				FundamentalFactory::getInstance().addFactory( iName , this);
			}

			FundamentalFieldFactory<T>& getInstance();

			virtual field* getField(int iAlign,cjson::dictionary* iDict) {
				return new fundamental_field<T>(iAlign, iDict);
			}

	};	

	#define CREATE_FUNDAMENTAL_TYPE_FACTORY(TYPE,NAME) const cjson::field::FundamentalFieldFactory<TYPE>* NAME##_instance = new cjson::field::FundamentalFieldFactory<TYPE>(); 
	#define CREATE_NAMED_FUNDAMENTAL_TYPE_FACTORY(TYPE,TNAME,NAME) const cjson::field::FundamentalFieldFactory<TYPE>* NAME##_instance = new cjson::field::FundamentalFieldFactory<TYPE>(TNAME); 
	
	


}}


#endif
