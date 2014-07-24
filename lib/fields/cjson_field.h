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
#ifndef _CJSON_FIELD_H_
#define _CJSON_FIELD_H_

#include <string>
#include <iostream>     
#include <sstream>
#include <stdlib.h>
#include "./jsonxx/jsonxx.h"      

//#define __DEBUG__

#define ENV_SIZE_BYTES sizeof(void*)
#define AD(TYPE,ENTRY) ((TYPE*)((long)ENTRY))
#define LD(TYPE,ENTRY,VALUE) *AD(TYPE,ENTRY) = (TYPE) VALUE

namespace cjson {

class dictionary;

namespace field {

	class field {

		protected:

			//int _offset;
			int _size;
			std::string _name;
			cjson::dictionary *_dictionary;

		public:

			field(const std::string& iName, cjson::dictionary* iDict){	
				_name = iName;
				_dictionary = iDict;
			}
			virtual ~field(){};

			const std::string& getName(){
				return _name;
			}

			const void setName(const std::string& iName){
				_name = iName;
			}
	
			void setSize(int iSize){
				_size = iSize;
			}	

			virtual int getSize(){
				return _size;
			}
			virtual void toJson(std::ostringstream& iStream, const void* iEntryPoint)=0;
			virtual void toJsonMap(std::ostringstream& iStream, const void* iEntryPoint){
				abort(); // not defined
			}
			virtual void* fromJson(const std::string& iJson)=0;
			virtual void fromJson(const jsonxx::Value& iJson, void* iEntryPoint)=0;
			
			virtual void fromJsonMap(const jsonxx::Value& iJson, void* iEntryPoint){
				abort(); // not defined
			}

			virtual void toCpp(std::ostringstream& iStream)=0;

			virtual bool less(const jsonxx::Value& iJsonL, const jsonxx::Value& iJsonR){
				abort(); // not defined
			}	

	};

	class field_value {
	
		public:	
		
			field* _field;
			jsonxx::Value _data;

		public:

			field_value(field* iField , const jsonxx::Value& iJson) {
				_field = iField;
				_data = iJson;
			}

			bool operator<(const field_value& rhs) const {
				return _field->less(_data,rhs._data);
			}

	}; 

     inline bool operator< (field_value &lhs, field_value &rhs){
		// TODO check same type between fields, to compare same things
		// even ... dump ... so abort and explain
		return lhs._field->less(lhs._data,rhs._data);
	}

	class attached_field { // for class or structs

		private:

			field* _innerField;
			unsigned long _offset;

		public:
	
		attached_field(const unsigned long iOffset, field* aField) {
			_offset = iOffset;
			_innerField = aField;
		}	

		~attached_field() {
			//delete _innerField;
		}			

		field* getField(){
			return _innerField;
		}

		const int getOffset(){
			return _offset;
		}

		const void setOffset(int iOffset){
			_offset = iOffset;
		}

		void shiftOffset(int iShift){
			_offset += iShift;
		}

	};


}}

#endif
