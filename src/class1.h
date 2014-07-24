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
#ifndef _CLASS1_H_
#define _CLASS1_H_

#include <stdio.h>
#include <string>
#include <vector>

namespace com {

	class classb {

		public :

			classb(){
				_ref="ref";
			}

			virtual void ident_bb(){
				printf("%s\n",_ref.c_str());
			}

			virtual void print(){
				printf("classb print\n");
			}

			virtual void print2(std::string& iStrRef){
				printf("print2 from b %s\n",iStrRef.c_str());
			}
			
		private :

			std::string _ref;

	};

	class F {
		
		public :

		virtual void printF(){
			printf("printF\n");
		}

		
	};

	class E {
		
		public :

		virtual void printE(){
			printf("printE\n");
		}

		
	};

	class D {

		virtual void printD(){
			printf("printD\n");
		}

	};

	class C  : public D {

		virtual void printC(){
			printf("printC\n");
		}

	};

	class B {

		virtual void printB(){
			printf("printB\n");
		}

	};	

	class A : public B, public C {

		virtual void printA(){
			printf("printA\n");
		}		
	
	};

	class classm1 : public classb, public  A {

		public :

			classm1(){
				_type="typem1";
			}

			virtual void ident_m1(){
				printf("TITI\n");
			}

			virtual void print2(std::string& iStrRef){
				printf("print2 from m1 %s\n",iStrRef.c_str());
			}				

		public :
			char* pdata;

		private :

			std::string _type;

	};

	class class0 {

		public :

			class0(){
				_type="type";
			}

			virtual void ident(){
				printf("TATA\n");
			}

			void setData(int i, int value) {
				data[i] = value;
			}

		private :

			std::string _type;
			int data[10];

	};


	class inner {

		public :

			std::string _ident;
			float pi;

			inner(){
				_ident = "test";
				pi = 3.14159265;
			}
	
	};

	class class1 : public classm1, public class0, public virtual E, public virtual F {

		public :

			class1(){
				_index=54;
				_name="first";
			}

			virtual void print(){
				printf("TOTO\n");
			}


		private :

			int _index;
			std::string _name;
			inner _inner;

	};

}

#endif



