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
/*
 *  Test program cjson
 *
 */

#include <stdio.h>
#include <iostream>

#include "./src/class1.h"

#include "cjson_dictionary.h"

using namespace std;

int main(int argc, char **argv)
{
   
	// Random class initialization
	com::class1* aObject = new com::class1();

	for (int i=0; i<10; i++){
		aObject->setData(i,i);
	}	

	aObject->pdata = new char[7];
	for (int i=0; i<7; i++){
		aObject->pdata[i] = 7-i;
	}
	
	// dictionary initialization
	cjson::dictionary aDict("./data/dictionary.xml");

	// json transformation
	std::string aJson = aDict.toJson<com::class1>(aObject);

	// print encoded class
	cout << aJson << std::endl ;

	// decode the object
	com::class1* aDecodedObject = aDict.fromJson<com::class1>(aJson);

	// modify data
	aDecodedObject->setData(4,22);

	// json transformation
	aJson = aDict.toJson<com::class1>(aDecodedObject);
	
	// print encoded class
	cout << aJson << std::endl ;

     return 0;

}
