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
#ifndef _CLASS02_H_
#define _CLASS02_H_

#include <stdio.h>
#include <string>
#include <vector>

namespace com02 {

	class A {
	  public:
  		virtual void f (){}
  		virtual void g (){}
  		virtual void h (){}
  		int ia;
	};

	class B: public virtual A {
	  public:
	  void f (){}
	  void h (){}
	  int ib;
	};

	class C: public virtual A {
	  public:
	  void g (){}
	  void h (){}
	  int ic;
	};

	class D: public B, public C {
	  public:
	  void h (){}
	  int id;
	};

	class X {
	  int ix;
	  virtual void x(){};
	};

	class E : public X, public D {
	  public:
	  int ie;
	  void f(){};
	  void h (){};
	};

	class S { virtual void f(){} };
     class T : virtual public S {};
     class U : virtual public T {};
     class V : public T, virtual public U {};


}


#endif
