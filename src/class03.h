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
#ifndef _CLASS03_H_
#define _CLASS03_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <map>

namespace com03 {

	class A {

		public:

			A(){
				for (int i=0; i<8; i++) {
					_vector.push_back(i*2);
				}
			}

			std::vector<int> _vector;

	};

	class inner {

		public:

			inner(){

				_name = "test";
				_number = 22;

			}

			std::string _name;
			int _number;

	};

	class B {

		public:

			B(){

				inner aInner;

				for (int i=0; i<8; i++) {
					_vector.push_back(aInner);
				}
			}

			std::vector<inner> _vector;

	};

	class C {

		public:

			C(){
				_map["A"]="B";
				_map["C"]="D";
				_map["E"]="F";
			}		

			std::map<std::string, std::string> _map;

	};

	class D {

		public:

			D(){
				_map[0]="B";
				_map[1]="D";
				_map[3]="F";
				_map[5]="K";
			}		

			std::map<int, std::string> _map;

	};

	class E {

		public:

			E(){
				_map["toto"]=inner();
				_map["tata"]=inner();
				_map["titi"]=inner();
				_map["tutu"]=inner();
			}		

			std::map<std::string, inner> _map;

	};

}

#endif

