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
#include <typeinfo>
#include <fstream>

#include "./src/class1.h"
#include "./src/class01.h"
#include "./src/class02.h"
#include "./src/class03.h"

#include "cjson_dictionary.h"
#include "./fields/cjson_class_field.h"

#include "demangle.h"

#include "./jsonxx/jsonxx.h"

using namespace std;

string textFileToString(string filename)
{
   string line;
   stringstream dosString;
   ifstream inFile;
   inFile.open (filename.c_str());
       while(getline(inFile, line))
        {           
           dosString<<line<<"\n";
        }
   inFile.close();
   return(dosString.str());
}

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void compareExpectedAndResult(string aExpected, string aResult){

	int aSize = aExpected.size()>aResult.size()?aExpected.size():aResult.size();
	for (int i=0; i<=aExpected.size()/(4*8); i++) {
		
		for (int j=0; j<4; j++) {

			for (int k=0; k<8; k++) {
			
				int aPos = i*32+j*8+k;
				if (aPos<aExpected.size()) {
					char aChar = aExpected[aPos];
					char aChar2 = 0;
					if (aPos<aResult.size()) {
						 aChar2 = aResult[aPos];
					}

					if (aChar>31 && aChar<128) {
						if (aChar==aChar2) {
							printf("%s %c%s",ANSI_COLOR_GREEN,aChar,ANSI_COLOR_RESET);
						} else {
							printf("%s %c%s",ANSI_COLOR_RED,aChar,ANSI_COLOR_RESET);
						}
					} else {
						if (aChar==aChar2) {
							printf("%s%02X%s", ANSI_COLOR_GREEN, aChar,ANSI_COLOR_RESET);
						} else {
							printf("%s%02X%s", ANSI_COLOR_RED, aChar,ANSI_COLOR_RESET);
						}
					}
				}

			}	
			printf("  ");
		}	
		printf("\n");

		for (int j=0; j<4; j++) {

			for (int k=0; k<8; k++) {
			
				int aPos = i*32+j*8+k;
				if (aPos<aResult.size()) {
					char aChar = aResult[aPos];
					if (aChar>31 && aChar<128) {
						printf("%s %c%s",ANSI_COLOR_YELLOW,aChar,ANSI_COLOR_RESET);
					} else {
						printf("%s%02X%s", ANSI_COLOR_YELLOW, aChar,ANSI_COLOR_RESET);
					}
				}

			}	
			printf("  ");
		}	
		printf("\n");


	}

}

bool compareFileAndString(string iFileName, string iContent){

	string aExpected = textFileToString(iFileName);
	aExpected = aExpected.substr(0,aExpected.size()-1);

	//printf ("String:%d Result:%d\n",(int) iContent.size(),(int) aExpected.size());

	if (aExpected==iContent) {
		printf("OK ");	
		return true;
	} else {
		printf("KO ");	
		compareExpectedAndResult(aExpected, iContent);

		printf("\n%s\n",iContent.c_str());
		return false;
	}

}

template<class T> void init(T* iObject){
}

template<class T> void post(T* iObject){
}

template<class T> void testCase(cjson::dictionary& iDict, string iFileName, const string& iJSonFileName = "", const string& iJSonFileName2 = "" , bool iDisplay= false){

	// Object initialization
	T* aObject = new T(); // the constructor have to exists

	init<T>(aObject);

	string aClassName = typeid(*aObject).name();	
	
	char aBuffer[2048];
	demangle(aBuffer, 2048, aClassName.c_str());

	printf("Try decoding %s ",aBuffer);
	cjson::field::field* aDataStruct = iDict.getDataStruct(std::string(aBuffer));

	if (iDisplay) {
		std::ostringstream aCppStream;	
		aDataStruct->toCpp(aCppStream);
		printf("C++ \n%s\n",aCppStream.str().c_str());

		printf("\n");
		for (int i=0; i<pointer_size(aObject)/8; i++) {
			char buffer[2048];
			unsigned long aVal = ((unsigned long*) ((long) aObject+i*8))[0];
			demangleFunctionPointer(buffer, 2048, (void *) aVal );
			printf("%02d [%p] %08x %08x %s \n", i,(void*) ((long) aObject+i*8), (int) ((aVal >> 32L) & 0x00000000FFFFFFFF) , (int) (aVal & 0x00000000FFFFFFFF), buffer );
		}

		printf("\n");
	}


	std::ostringstream aStream;
	aDataStruct->toJson(aStream, aObject);
	std::string aJson = aStream.str();

	if (iJSonFileName=="") {
		printf("Json object : %s\n",aJson.c_str());
	} else {
		compareFileAndString(iJSonFileName, aJson);
	}	

	cjson::field::class_field* aClassSP = dynamic_cast<cjson::field::class_field*>(aDataStruct);

	stringstream ass;	
	aClassSP->dumpVTable(ass, true);
	string aResult = ass.str();
	
	compareFileAndString(iFileName, aResult);

	// Second part of the test
	T* aDecodedObject = reinterpret_cast<T*> (aDataStruct->fromJson(aJson));

	if (iDisplay) {
		for (int i=0; i<pointer_size(aDecodedObject)/8; i++) {
			char buffer[2048];
			unsigned long aVal = ((unsigned long*) ((long) aDecodedObject+i*8))[0];
			demangleFunctionPointer(buffer, 2048, (void *) aVal );
			printf("%02d [%p] %08x %08x %s \n", i,(void*) ((long) aObject+i*8), (int) ((aVal >> 32L) & 0x00000000FFFFFFFF) , (int) (aVal & 0x00000000FFFFFFFF), buffer );
		}

		stringstream ass;	
		aClassSP->dumpVTable(ass);
		printf("%s\n",ass.str().c_str());		
	}

	// This demonstrates that built objects are usable
	post<T>(aDecodedObject);

	std::ostringstream aStreamDecode;
	aDataStruct->toJson(aStreamDecode, aDecodedObject);
	aJson = aStreamDecode.str();

	if ((iJSonFileName=="") && (iJSonFileName2=="")) {
		printf("Json object : %s\n",aJson.c_str());
	} else if (iJSonFileName2!="") {
		compareFileAndString(iJSonFileName2, aJson);
	} else if (iJSonFileName!="") {
		compareFileAndString(iJSonFileName, aJson);
	}

	printf("\n");

}

template<> void init(com::class1* iObject){

	for (int i=0; i<10; i++){
		iObject->setData(i,i);
	}	

	iObject->pdata = new char[7];
	for (int i=0; i<7; i++){
		iObject->pdata[i] = 7-i;
	}	

}

template<> void init(com02::D* iObject){

	iObject->ia = 1;
	iObject->ib = 2;
	iObject->ic = 3;
	iObject->id = 4;

}

template<> void init(com02::E* iObject){

	iObject->ia = 1;
	iObject->ib = 2;
	iObject->ic = 3;
	iObject->id = 4;
	iObject->ie = 5;

}

template<> void init(com02::B* iObject){

	iObject->ia = 1;
	iObject->ib = 2;

}

template<> void post(com03::A* iObject){

	iObject->_vector.push_back(17);

}

template<> void post(com03::C* iObject){

	iObject->_map["G"] = "H";

}

template<> void post(com03::D* iObject){

	iObject->_map[8] = "I";

}

template<> void post(com03::E* iObject){

	iObject->_map["postadded"] = com03::inner();

}

int main(int argc, char **argv)
{
   
	cjson::dictionary aDict("./data/dictionary.xml");

	testCase<com::class1>(aDict,"data/unittest/com_class1.txt","data/unittest/com_class1.json","data/unittest/com_class1.json");

	// class01.h

	testCase<com01::class0>(aDict,"data/unittest/com01_class0.txt","data/unittest/com01_class0.json");
	testCase<com01::class1>(aDict,"data/unittest/com01_class0.txt","data/unittest/com01_class1.json");
	testCase<com01::class2>(aDict,"data/unittest/com01_class2.txt","data/unittest/none.json");
	testCase<com01::class3>(aDict,"data/unittest/com01_class3.txt","data/unittest/none.json");
	testCase<com01::class4>(aDict,"data/unittest/com01_class4.txt","data/unittest/none.json");
	testCase<com01::class5>(aDict,"data/unittest/com01_class5.txt","data/unittest/none.json");
	testCase<com01::class6>(aDict,"data/unittest/com01_class6.txt","data/unittest/com01_class6.json");
	testCase<com01::class7>(aDict,"data/unittest/com01_class7.txt","data/unittest/none.json");
	testCase<com01::class8>(aDict,"data/unittest/com01_class8.txt","data/unittest/none.json");

	// class02.h
	
	testCase<com02::B>(aDict,"data/unittest/com02_B.txt","data/unittest/com02_B.json");
	testCase<com02::C>(aDict,"data/unittest/com02_C.txt","data/unittest/com02_C.json");
	testCase<com02::D>(aDict,"data/unittest/com02_D.txt","data/unittest/com02_D.json");
	testCase<com02::E>(aDict,"data/unittest/com02_E.txt","data/unittest/com02_E.json");
	testCase<com02::V>(aDict,"data/unittest/com02_V.txt","data/unittest/none.json");

	// class03.h

	testCase<com03::A>(aDict,"data/unittest/com03_A.txt","data/unittest/com03_A.json","data/unittest/com03_A_post.json");
	testCase<com03::B>(aDict,"data/unittest/com03_B.txt","data/unittest/com03_B.json");
	testCase<com03::C>(aDict,"data/unittest/com03_C.txt","data/unittest/com03_C.json", "data/unittest/com03_C_post.json");	
	testCase<com03::D>(aDict,"data/unittest/com03_D.txt","data/unittest/com03_D.json", "data/unittest/com03_D_post.json");
	testCase<com03::E>(aDict,"data/unittest/com03_E.txt","data/unittest/com03_E.json", "data/unittest/com03_E_post.json");	

}
