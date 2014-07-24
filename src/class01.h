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
#ifndef _CLASS01_H_
#define _CLASS01_H_

#include <stdio.h>
#include <string>
#include <vector>

namespace com01 {

	class class0 {

		public:

		class0(){
			i = 26;
		}

		private:

		int i;

	};

	class class1 : public class0 {

		public:

		class1(){
			j = 1;
		}

		private:

		int j;

	};

	class class2 {

		public:

		class2(){
		
		}

		virtual void print(){}		

	};

	class class3 : public class2 {

		public:

		class3(){
		
		}	

	};

	class class4 : public class2 {

		public:

		class4(){
		
		}	

		virtual void print(){}		

	};

	class class5 : public virtual class2 {

		public:

		class5(){
		
		}			

	};

	class class6 : public virtual class2, public virtual class0 {

		public:

		class6(){
		
		}			

	};

	class class7 : public class5 {

		public:

		class7(){
		
		}	

	};

	class class8 : public class5, public class2 {

		public:

		class8(){
		
		}	

	};

}

#endif
