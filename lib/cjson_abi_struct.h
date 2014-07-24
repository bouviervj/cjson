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
#ifndef _CJSON_ABI_STRUCT_H_
#define _CJSON_ABI_STRUCT_H_

#define INCVOID(POINTER)  POINTER=((void*) (((long) POINTER)+sizeof(void*)))

#include <stdio.h>
#include <algorithm>

namespace cjson {

	#define ABI_UNKNOWN       0
	#define ABI_OFFSET_TO_TOP 1
	#define ABI_RTTI          2
	#define ABI_FUNCTION      3
	#define ABI_VCALL_OFFSET  4
	#define ABI_VTBLE_OFFSET  5


	typedef struct ABI_Layout_Element {
		int _type;
		std::string _class; 		
		std::string _name;
		void* _address;
		long _value;
	} ABI_LElem;

	typedef struct ABI_Method_Element { 		
		std::string _mangled;
		std::string _name;
		std::string _class;
		std::vector<std::string> _arguments;

		void dump(){
			printf("method: %s\n",_mangled.c_str());
			if (_arguments.size()>0) {
				printf("arguments:\n");
				for (int i=0; i<_arguments.size(); i++){
					printf("%s\n",_arguments[i].c_str());
				}
			}
		}

		friend bool operator == (const ABI_Method_Element& lhs, const ABI_Method_Element& rhs) { 
#ifdef __DEBUG_ABI__
			printf(" %s vs %s \n",lhs._name.c_str(), rhs._name.c_str());
#endif
			if ((lhs._name != rhs._name)) return false;
			if (lhs._arguments.size()!=rhs._arguments.size()) return false;
			for (int i=0; i<lhs._arguments.size(); i++) {
				if (lhs._arguments[i]!=rhs._arguments[i]) return false;
			}
			return true; 
		}

	} ABI_MElem;
	
	typedef struct ABI_Inheritance_Struct {
		std::string _name;
		bool _virtual_inherited;
		bool _empty;
		std::vector<ABI_Method_Element>     _virtual_methods;
		std::vector<ABI_Inheritance_Struct> _inherited;
		
		void dump(){
			printf("class/struct: %s\n",_name.c_str());
			printf("virtual inherited: %d\n",_virtual_inherited);

			if (_virtual_methods.size()>0) {
				printf("virtual methods:\n");
				for (int i=0; i<_virtual_methods.size(); i++){
					//printf("%s\n",_virtual_methods[i]._mangled.c_str());
					_virtual_methods[i].dump();
				}
			}
			
			if (_inherited.size()>0) {
				printf("inherits:\n");
				for (int i=0; i<_inherited.size(); i++){
					_inherited[i].dump();
				}
			}

		}

		void extractInheritedDynamicList(std::vector<std::string>& ioInhDynList){

			if (_inherited.size()>0) {
				std::vector<std::string>::iterator it;
				for (int i=0; i<_inherited.size(); i++){
  					it = find (ioInhDynList.begin(), ioInhDynList.end(), _inherited[i]._name);
					if (it==ioInhDynList.end()) {
						ioInhDynList.push_back(_inherited[i]._name);
					}
					_inherited[i].extractInheritedDynamicList(ioInhDynList);
				}
			}

		}
		
		void addOffsetToTop(std::vector<ABI_Layout_Element>& iLayout,ABI_Inheritance_Struct& iRef, void*& iVTAddress){

			ABI_Layout_Element aOffToTop;
			aOffToTop._type = ABI_OFFSET_TO_TOP;
			aOffToTop._class = iRef._name;
			aOffToTop._name = "offset to top";
			aOffToTop._address = iVTAddress;
			INCVOID(iVTAddress);
			iLayout.push_back(aOffToTop);
	
		}

		void addRTTI(std::vector<ABI_Layout_Element>& iLayout,ABI_Inheritance_Struct& iRef, void*& iVTAddress){

			ABI_Layout_Element aRTTI;
			aRTTI._type = ABI_RTTI;
			aRTTI._class = _name;
			aRTTI._name = "rtti";
			aRTTI._address = iVTAddress;	
			INCVOID(iVTAddress);
			iLayout.push_back(aRTTI);
	
		}

		void addFunction(std::vector<ABI_Layout_Element>& iLayout,void*& iVTAddress, ABI_Method_Element& iFunctionDesc){

			ABI_Layout_Element aFunction;
			aFunction._type = ABI_FUNCTION;
			aFunction._class = iFunctionDesc._class;
			aFunction._name = iFunctionDesc._mangled;					
			aFunction._address = iVTAddress;
			INCVOID(iVTAddress);
			iLayout.push_back(aFunction);
	
		}

		void addVCallOffset(std::vector<ABI_Layout_Element>& iLayout,void*& iVTAddress, ABI_Method_Element& iFunctionDesc){

			ABI_Layout_Element aFunction;
			aFunction._type = ABI_VCALL_OFFSET;
			aFunction._class = iFunctionDesc._class;
			aFunction._name = "offset to " + iFunctionDesc._name;					
			aFunction._address = iVTAddress;
			INCVOID(iVTAddress);
			iLayout.push_back(aFunction);
	
		}

		void addVTbleOffset(std::vector<ABI_Layout_Element>& iLayout,void*& iVTAddress){

			ABI_Layout_Element aFunction;
			aFunction._type = ABI_VTBLE_OFFSET;
			aFunction._class = _name;
			aFunction._name = "vtable offset";					
			aFunction._address = iVTAddress;
			INCVOID(iVTAddress);
			iLayout.push_back(aFunction);
	
		}

		void addFunctionWithInheritedRules(std::vector<ABI_Method_Element>& iFunctions, ABI_Method_Element& iFunctionDesc){
			std::vector<ABI_Method_Element>::iterator it;
			it = find(iFunctions.begin(), iFunctions.end(), iFunctionDesc);
			if (it!=iFunctions.end()) {
#ifdef __DEBUG_ABI__
				printf("replace %s\n",iFunctionDesc._name.c_str());
#endif
				*it = iFunctionDesc;
				return;
			}
#ifdef __DEBUG_ABI__
			printf("add %s\n",iFunctionDesc._name.c_str());
#endif

			iFunctions.push_back(iFunctionDesc);
		}

		void addStructFunctionWithInhRules(std::vector<ABI_Method_Element>& iFunctions, ABI_Inheritance_Struct& iStruct, bool iInherited = false){
			
			if (iInherited) {
	
				for (int i=0;i<iStruct._inherited.size(); i++){
					addStructFunctionWithInhRules(iFunctions, iStruct._inherited[i], true);
				}

			}
			for (int i=0; i<iStruct._virtual_methods.size(); i++) {
				addFunctionWithInheritedRules(iFunctions, iStruct._virtual_methods[i]);
			}
			
		}

		bool isTrivial(){
			return (!_virtual_inherited) && (_inherited.size()==0) && (_virtual_methods.size()==0);
		}

		bool isLeaf(){
			return (!_virtual_inherited) && (_inherited.size()==0) && (_virtual_methods.size()>0);
		}

		bool inheritsVirtualMethods(){
			for (int i=0; i<_inherited.size(); i++) {
				if (_inherited[i]._virtual_methods.size()>0) return true;
				if (_inherited[i].inheritsVirtualMethods()) return true;
			}
			return false;
		}

		bool isNonVirtualBasesOnly(){			
			for (int i=0; i<_inherited.size(); i++) {
				if (_inherited[i]._virtual_inherited) return false;
			}
			return inheritsVirtualMethods();
		}

		bool isVirtualBasesOnly(){
			for (int i=0; i<_inherited.size(); i++) {
				if (!_inherited[i]._virtual_inherited) return false;
			}
			return true;
		}

		bool isComplex(){
			return (!isTrivial()) && (!isNonVirtualBasesOnly()) && (!isVirtualBasesOnly());
		}

		bool isNearlyEmpty(){
		
			if (_empty) { // no fields
				
				if ((_inherited.size()==0) && (_virtual_methods.size()>0)) { // has vtable
					return true;
				} else {

					for (int i=0; i<_inherited.size(); i++){
						if (!(_inherited[i].isNearlyEmpty()))    {
							return false;
						}
					}

					return true;

				}
				
			}

			return false; 

		}
		
		bool hasVirtualMethods(){
			if (_virtual_methods.size()>0) {
				return true;
			} else {
				if (_inherited.size()>0) {
					for (int i=0; i<_inherited.size(); i++) {	
						if (_inherited[i].hasVirtualMethods()) return true;
					}
				}
			}
			return false;
		}

		// A primary base is the first non-virtual class with virtual functions or, if none, a first nearly empty virtual base or, if none, a first virtual base.
		void findPrimaryChain(std::vector<std::string>& ioPrimaryVirtualBase,std::map<std::string,ABI_Inheritance_Struct>& ioPrimaryMap){
		
			// Try to find first non-virtual class with virtual functions
			for (int i=0; i<_inherited.size(); i++){
				if (!_inherited[i]._virtual_inherited) {  // Not virtually inherited
					if (_inherited[i].hasVirtualMethods()>0) {
#ifdef __DEBUG_ABI__
						printf("not virtually inherited : %s\n",_inherited[i]._name.c_str());	
#endif	
						_inherited[i].findPrimaryChain(ioPrimaryVirtualBase,ioPrimaryMap);
						ioPrimaryVirtualBase.push_back(_inherited[i]._name); // POST push
						ioPrimaryMap[_inherited[i]._name]=_inherited[i];
						return; // exit don't need to go further
					} 
				} 
			}

			// Try to find the first nearly empty virtual base
			for (int i=0; i<_inherited.size(); i++){
				if (_inherited[i]._virtual_inherited) {  // Virtually inherited
					if (_inherited[i].isNearlyEmpty()) {  // empty of fields and nee	
#ifdef __DEBUG_ABI__
						printf("nearly empty inherited : %s\n",_inherited[i]._name.c_str());
#endif	
						_inherited[i].findPrimaryChain(ioPrimaryVirtualBase,ioPrimaryMap);
						ioPrimaryVirtualBase.push_back("V/"+_inherited[i]._name); // POST push
						ioPrimaryMap["V/"+_inherited[i]._name]=_inherited[i];
						return; // exit don't need to go further
					} 
				} 
			}

			// Try to find the first virtual base
			for (int i=0; i<_inherited.size(); i++){
				if (_inherited[i]._virtual_inherited) {  // Virtually inherited
#ifdef __DEBUG_ABI__
					printf("virtual inherited : %s\n",_inherited[i]._name.c_str());	
#endif
					_inherited[i].findPrimaryChain(ioPrimaryVirtualBase,ioPrimaryMap);
					ioPrimaryVirtualBase.push_back("V/"+_inherited[i]._name); // POST push
					ioPrimaryMap["V/"+_inherited[i]._name]=_inherited[i];
					return; 
				} 
			}

		}

		void findSecondary(std::vector<std::string>& ioPrimaryVirtualBase,
					    std::vector<std::string>& ioSecondaryVirtualBase, 
					    std::map<std::string,ABI_Inheritance_Struct>& ioPrimaryMap, 
					    std::map<std::string,ABI_Inheritance_Struct>& ioSecondaryMap) {

			for (int i=0; i<ioPrimaryVirtualBase.size(); i++){
				
				//_inherited[i].findSecondary(ioSecondaryVirtualBase,ioSecondaryMap,ioPrimaryMap);
				ABI_Inheritance_Struct& aPStruct = ioPrimaryMap[ioPrimaryVirtualBase[i]];
#ifdef __DEBUG_ABI__
				printf("##primary:%s\n",aPStruct._name.c_str());
#endif
				for (int j=0; j<aPStruct._inherited.size(); j++){

					std::string aName = aPStruct._inherited[j]._name;
					if (aPStruct._inherited[j]._virtual_inherited) {
						aName = "V/"+aName;
					}

					if (ioPrimaryMap.find(aName)==ioPrimaryMap.end()) {						
						ioSecondaryVirtualBase.push_back(aName); // POST push
						ioSecondaryMap[aName]= aPStruct._inherited[j];
					} 

				}
			}

			// Adding own inherited
			for (int j=0; j<_inherited.size(); j++){

				std::string aName = _inherited[j]._name;
				if (_inherited[j]._virtual_inherited) {
					aName = "V/"+aName;
				}

				if (ioPrimaryMap.find(aName)==ioPrimaryMap.end()) {						
					ioSecondaryVirtualBase.push_back(aName); // POST push
					ioSecondaryMap[aName]=_inherited[j];
				} 

			}
			

		}

		void findPrimarySecondaryBase(std::vector<std::string>& ioPrimaryVirtualBase,
								std::vector<std::string>& ioSecondaryVirtualBase, 
								std::map<std::string,ABI_Inheritance_Struct>& ioPrimaryMap,
								std::map<std::string,ABI_Inheritance_Struct>& ioSecondaryMap,
								bool forceSecondary = false){

			findPrimaryChain(ioPrimaryVirtualBase,ioPrimaryMap);
			findSecondary(ioPrimaryVirtualBase,ioSecondaryVirtualBase,ioPrimaryMap,ioSecondaryMap);
			
		}

		void findInheritedVirtualBases(std::vector<std::string >& iVirtualBase, std::map<std::string , ABI_Inheritance_Struct >& iVMap, int iDepth=0){
			
			if (_inherited.size()>0) {
				for (int i=0; i<_inherited.size(); i++) {
					_inherited[i].findInheritedVirtualBases(iVirtualBase,iVMap, iDepth+1);
				}
			}	

			if ((_virtual_inherited) && /*(_virtual_methods.size()>0) &&*/ (iDepth!=0)) { //not myself

				if (iVMap.find(_name)==iVMap.end()) {
					iVirtualBase.push_back(_name);
					iVMap[_name]= *this;
				}

			}

		}		
		
		void buildVirtualTableLayout(std::vector<ABI_Layout_Element>& iLayout, std::map<std::string,void*>& iVTableEntryPoint, void*& iVTAddress){

			std::vector<std::string > aVirtualBase;
			std::map<std::string , ABI_Inheritance_Struct > aVMap;

			buildVirtualTableLayout(iLayout, iVTableEntryPoint, iVTAddress, aVirtualBase, aVMap);

		}


		void buildVirtualTableLayout(std::vector<ABI_Layout_Element>& iLayout,
							    std::map<std::string,void*>& iVTableEntryPoint, 
							    void*& iVTAddress, 
							    std::vector<std::string >& iVirtualBase,
							    std::map<std::string , ABI_Inheritance_Struct >& iVMap,
							    int iDepth = 0){

			if (isTrivial()) return;

			if (isLeaf()) { // Cat 1
				
				addOffsetToTop(iLayout,*this,iVTAddress);
				addRTTI(iLayout,*this,iVTAddress);
			
				iVTableEntryPoint[_name]=iVTAddress;
				for (int i=0; i<_virtual_methods.size(); i++) {
					addFunction(iLayout,iVTAddress,_virtual_methods[i]);
				}
				return;
		
			}
			
			//if (isNonVirtualBasesOnly() || isComplex()) { // Cat 2
#ifdef __DEBUG_ABI__			
			printf("class %s depth %d\n",_name.c_str(),iDepth);
#endif

			std::vector<std::string > aPrimaryVirtualBase;
			std::vector<std::string > aSecondaryVirtualBase;
			std::map<std::string , ABI_Inheritance_Struct > aPMap;
			std::map<std::string , ABI_Inheritance_Struct > aSMap;
			findPrimarySecondaryBase(aPrimaryVirtualBase,aSecondaryVirtualBase,aPMap,aSMap);
			
			std::vector<std::string > iVirtualBaseInit;
			std::map<std::string , ABI_Inheritance_Struct > aVMap;

			findInheritedVirtualBases(iVirtualBaseInit, aVMap);
#ifdef __DEBUG_ABI__
			printf("virtual base number %d for base %s\n",(int) iVirtualBaseInit.size(), _name.c_str());
#endif
			if (iVirtualBaseInit.size()>0) { // this inherits from virtual bases

				for (int i=0; i<iVirtualBaseInit.size(); i++) {
#ifdef __DEBUG_ABI__
					printf("virtual base : %s\n",iVirtualBaseInit[i].c_str());
#endif
					addVTbleOffset(iLayout,iVTAddress);
				}

			}
			// know we can really build virtual inherited table
			

			if (aPrimaryVirtualBase.size()>0) {

				ABI_Inheritance_Struct& aStruct = aPMap[aPrimaryVirtualBase[0]];					

				bool virtualmode = false;
				int aPosLastVirt = 0;
				// Offset function list
				std::vector<ABI_MElem> aStFunctionList;
				for (int i=0; i<aPrimaryVirtualBase.size(); i++) {
					ABI_Inheritance_Struct& aPStruct = aPMap[aPrimaryVirtualBase[i]];
					// if primary virtual and nearly empty, directly add function offsets
					if (aPStruct._virtual_inherited) { 

						  aPosLastVirt = i+1;

						  if (aPStruct.isNearlyEmpty()) {
							
							addStructFunctionWithInhRules(aStFunctionList, aPStruct , true);		
						
							virtualmode = true;	
						  }


				     }
				}
				for (int j=0; j<aStFunctionList.size(); j++) {
					addVCallOffset(iLayout,iVTAddress, aStFunctionList[j]);
				}

				// Normal class
				addOffsetToTop(iLayout,aStruct,iVTAddress);
				addRTTI(iLayout,*this,iVTAddress);

				//printf("name: %s\n",aStruct._name.c_str());
				iVTableEntryPoint[_name]=iVTAddress;
				// Here we prepare a vector of inherited methods
				std::vector<ABI_MElem> aFunctionList;
				int start = 0;
				if (!virtualmode) start=aPosLastVirt;
				for (int i=start; i<aPrimaryVirtualBase.size(); i++) {
				    ABI_Inheritance_Struct& aPStruct = aPMap[aPrimaryVirtualBase[i]];
#ifdef __DEBUG_ABI__
                        printf("#### %s -> %s \n",_name.c_str(),aPStruct._name.c_str());
#endif
				    addStructFunctionWithInhRules(aFunctionList, aPStruct);
				}
				addStructFunctionWithInhRules(aFunctionList, *this);

				//printf("number function to print %d\n",aFunctionList.size()); 
				for (int i=0; i<aFunctionList.size(); i++) {
					addFunction(iLayout,iVTAddress, aFunctionList[i]);
				}

				if ((!virtualmode) && (aStruct._virtual_inherited)) {
#ifdef __DEBUG_ABI__					
					printf("Test %s\n",aStruct._name.c_str()); 
#endif
					if ((iVMap.find(aStruct._name)==iVMap.end()) && (iDepth!=-1)) {
#ifdef __DEBUG_ABI__
						printf("added %s\n",aStruct._name.c_str());
#endif 
						iVirtualBase.push_back(aStruct._name);
						iVMap[aStruct._name]=aStruct;	
					}

				}

			} else {
			
				if (_virtual_inherited) {
				
					std::vector<ABI_MElem> aFunctionList;
					addStructFunctionWithInhRules(aFunctionList, *this , true);	

					for (int j=0; j<aFunctionList.size(); j++) {
						addVCallOffset(iLayout,iVTAddress, aFunctionList[j]);
					}	
							
				}

				if (hasVirtualMethods()) {
					addOffsetToTop(iLayout, *this,iVTAddress);
					addRTTI(iLayout,*this,iVTAddress);

					//printf("name: %s\n",aStruct._name.c_str());
					iVTableEntryPoint[_name]=iVTAddress;
					// Here we prepare a vector of inherited methods
					std::vector<ABI_MElem> aFunctionList;
					addStructFunctionWithInhRules(aFunctionList, *this);

					//printf("number function to print %d\n",aFunctionList.size()); 
					for (int i=0; i<aFunctionList.size(); i++) {
						addFunction(iLayout,iVTAddress, aFunctionList[i]);
					}
				}

			}
			
			if (aSecondaryVirtualBase.size()>0) {
				
				std::vector<ABI_Inheritance_Struct> aVirtual;
				for (int i=0; i<aSecondaryVirtualBase.size(); i++){
					ABI_Inheritance_Struct& aStruct = aSMap[aSecondaryVirtualBase[i]];
#ifdef __DEBUG_ABI__
					printf("#build secondary class:%s\n",aSecondaryVirtualBase[i].c_str());
#endif
					aStruct.buildVirtualTableLayout(iLayout, iVTableEntryPoint, iVTAddress, iVirtualBase, iVMap, iDepth+1);
				}

			}
				
			if ((iVirtualBase.size()>0) && (iDepth==0)) {
				for (int i=0; i<iVirtualBase.size(); i++) {
#ifdef __DEBUG_ABI__
					printf(">>> %s\n",iVirtualBase[i].c_str());
#endif 
					iVMap[iVirtualBase[i]].buildVirtualTableLayout(iLayout, iVTableEntryPoint, iVTAddress, iVirtualBase, iVMap, -1);
				}
			} 
#ifdef __DEBUG_ABI__
			  else {
				if  (iDepth==0) printf(">>>> no end vbase %s\n",_name.c_str()); 
			}	
#endif			

		}

	} ABI_Inh;

}

#endif
