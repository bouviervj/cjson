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
#include "cjson_std_map_field.h"

#include "../cjson_dictionary.h"

#include "cjson_regexp.h"

using namespace jsonxx;
  
namespace cjson {

namespace field {

	// std::map<std::string, std::string, std::less<std::string>, std::allocator<std::pair<std::string const, std::string> > >
	
	#define REG_EXP_STD_MAP "^std::map<(.*),\\s(.*),\\sstd::less<(.*)>,\\sstd::allocator<std::pair<(.*)>\\s>\\s>$"

	CREATE_FIELD_TYPE_FACTORY( REG_EXP_STD_MAP ,std_map_field)

	unsigned long std_map_field::getAlignedSize(unsigned long iSize){
		unsigned long  aAlignedSize = iSize;
		if (aAlignedSize % (ENV_SIZE_BYTES*8)!=0) {   // 64 bits
			aAlignedSize = (aAlignedSize/(ENV_SIZE_BYTES*8)+1)*(ENV_SIZE_BYTES*8);
		}
		return aAlignedSize;
	}

	void std_map_field::decodeKeyValuePair(std::ostringstream& iStream, std::_Rb_tree_node_base* iNode, field* iFirst, field* iSecond){

		std::_Rb_tree_node<long>* aNode = (std::_Rb_tree_node<long>*) iNode;	

		if (aNode) {	
			void* aKeyAddress = (void*) (&aNode->_M_value_field); // In fact here we get the address of key 
													    // Here the value  is std::pair < typeKey, typeValue >
													    // !!! Dependant of std::pair implementation
			
			//  Here we have padding in structs expect alignment on 64bits .... arghhh   tr1 can help may be
			int aAlignedSize = getAlignedSize(iFirst->getSize());
#ifdef __DEBUG__
			printf("Aligned size for %s : %d\n", iFirst->getName().c_str(), aAlignedSize );
#endif			
			void* aValueAddress = (void*) ((long) aKeyAddress + aAlignedSize/8);	
			iFirst->toJsonMap(iStream, aKeyAddress);
			iStream << ":";
			iSecond->toJson(iStream, aValueAddress);		

		}

	}

	void std_map_field::readRedBlackTree(std::vector<std::string>& iElementsList , std::_Rb_tree_node_base* iNode, field* iFirst, field* iSecond){

		if (iNode) {

			std::ostringstream aKeyValueStream;	
			decodeKeyValuePair(aKeyValueStream, iNode, iFirst, iSecond);
			iElementsList.push_back(aKeyValueStream.str());
		
			readRedBlackTree(iElementsList , iNode->_M_left, iFirst, iSecond);
			readRedBlackTree(iElementsList , iNode->_M_right, iFirst, iSecond);

		}

	}

	void std_map_field::init(){
		
		_init = true;

		regexp aRegexp(REG_EXP_STD_MAP);
		if (aRegexp.matches(_name)) {

			_first = _dictionary->getDataStruct(aRegexp.get(1));
#ifdef __DEBUG__
			if (_first) printf("first selected ok #%s#\n",aRegexp.get(1).c_str());
#endif
			_second = _dictionary->getDataStruct(aRegexp.get(2));
#ifdef __DEBUG__
			if (_second) printf("second selected ok #%s#\n",aRegexp.get(2).c_str());
#endif

		}

	}

	void std_map_field::toJson(std::ostringstream& iStream, const void* iEntryPoint){

		void* aAddress = (void*) iEntryPoint;

		std::_Rb_tree_node_base* aBase = (std::_Rb_tree_node_base*) ((long) aAddress + ENV_SIZE_BYTES);
#ifdef __DEBUG__
		printf("aBase %p\n",aBase->_M_parent);
#endif

		std::_Rb_tree_node<long>* aNode = (std::_Rb_tree_node<long>*) aBase->_M_parent;	
		
		// Here init once the field
		if (!_init) init();

		iStream << "{";
		std::vector<std::string> aElements;
		readRedBlackTree(aElements, aNode, _first, _second);
		for (int i=0;i <aElements.size(); i++){
			iStream << aElements[i];
			if (i<aElements.size()-1) iStream << ",";
		}
		iStream << "}";

	}

	void std_map_field::toJsonMap(std::ostringstream& iStream, const void* iEntryPoint){
		abort(); // not implemented
	}
	
	void* std_map_field::fromJson(const std::string& iJson){
	// decodes a single char
	// TODO to implement
	}

	typedef std::_Rb_tree_node<std::pair<field_value, jsonxx::Value> > treenode_def;
	
	void std_map_field::copyNode(treenode_def* iNodeToCopy, field* iSecond, void* iParent, void*& ioPointer, void*& iFirst, void*& iLast){

		if (iNodeToCopy!=0) {

#ifdef __DEBUG__
			printf("#################\n");
			printf("Node this %p\n",iNodeToCopy);
			printf("Node color %d\n",iNodeToCopy->_M_color);
			printf("Node parent %p\n",iNodeToCopy->_M_parent);
			printf("Node left  %p\n",iNodeToCopy->_M_left);
			printf("Node right %p\n",iNodeToCopy->_M_right);
#endif

			field_value aFValue = iNodeToCopy->_M_value_field.first;
			unsigned long aKeySize = getAlignedSize(aFValue._field->getSize());
			unsigned long aValueSize =getAlignedSize(iSecond->getSize());
			unsigned long aItemSize = sizeof(std::_Rb_tree_node_base)+
								 aKeySize/8+ // Start pair size coding
								 aValueSize/8;

			// Allocate memory for the node
		     ioPointer = malloc(aItemSize);
			std::_Rb_tree_node_base* aBase = (std::_Rb_tree_node_base*)	ioPointer;

			// Building node
			aBase->_M_color = iNodeToCopy->_M_color; // Copy color
			void* aLeft = 0, *aRight = 0;
			copyNode((treenode_def*) iNodeToCopy->_M_left, iSecond, ioPointer, aLeft,  iFirst, iLast);  // Copy left
			copyNode((treenode_def*) iNodeToCopy->_M_right, iSecond, ioPointer, aRight, iFirst, iLast); // Copy right
			aBase->_M_left = (std::_Rb_tree_node_base*) aLeft;
			aBase->_M_right = (std::_Rb_tree_node_base*) aRight;
			aBase->_M_parent = (std::_Rb_tree_node_base*) iParent;

			// Then we try to build pair values
			void* aKeyAddress = (void*) (aBase+1);
			aFValue._field->fromJsonMap(aFValue._data, aKeyAddress);

#ifdef __DEBUG__		
			//printf("first field offset:%d\n",aFValue._field->getOffset());
			//printf("second field offset:%d\n",iSecond->getOffset());
#endif

			void* aValueAddress = (void*) ((long) aKeyAddress + aKeySize/8);
#ifdef __DEBUG__
			printf("Second type:%d\n", iNodeToCopy->_M_value_field.second.type_ );
#endif
			iSecond->fromJson(iNodeToCopy->_M_value_field.second, aValueAddress);

			if (iFirst==(void*) iNodeToCopy) iFirst = ioPointer;
			if (iLast==(void*) iNodeToCopy) iLast = ioPointer;

#ifdef __DEBUG__
			printf("**\n");
			printf("Copied Node this %p\n",aBase);
			printf("Copied Node color %d\n",aBase->_M_color);
			printf("Copied Node parent %p\n",aBase->_M_parent);
			printf("Copied Node left  %p\n",aBase->_M_left);
			printf("Copied right %p\n",aBase->_M_right);
			
			for (int i=0; i<pointer_size(ioPointer)/8; i++) {
				char buffer[2048];
				unsigned long aVal = ((unsigned long*) ((long) ioPointer+i*ENV_SIZE_BYTES))[0];
				demangleFunctionPointer(buffer, 2048, (void *) aVal );
				if (ENV_SIZE_BYTES==4) {
					printf("%02d [%p] %08x %s \n", i,(void*) ((long) ioPointer+i*ENV_SIZE_BYTES), (unsigned int) aVal, buffer );
				} else {
					printf("%02d [%p] %08x %08x %s \n", i,(void*) ((long) ioPointer+i*ENV_SIZE_BYTES), (int) ((aVal >> 32L) & 0x00000000FFFFFFFF) , (int) (aVal & 0x00000000FFFFFFFF), buffer );
				}
			}			

			printf("#################\n");
#endif
			
			return;			

		}

		ioPointer = 0;

	}	


	void std_map_field::fromJson(const jsonxx::Value& iJson, void* iEntryPoint){

		if (!_init) init();

		void* aAddress = (void*) iEntryPoint;

		std::vector<std::pair<jsonxx::Value, jsonxx::Value> > aKeyValues;
		iJson.get<Object>().getKeyValues(aKeyValues);	
		
		std::map<field_value,jsonxx::Value> _building_map;
	
		for (int i=0;i<aKeyValues.size(); i++) {
				
			field_value aVal(_first, aKeyValues[i].first);
#ifdef __DEBUG__
			printf("Sec type:%d\n", aKeyValues[i].second.type_ );
#endif
			_building_map[aVal] = aKeyValues[i].second;
		
		}
		
		// We have now in this map a build representation of our final map
		// We have to copy its structure		
		std::_Rb_tree_node_base* aBase = (std::_Rb_tree_node_base*) ((long) &_building_map + ENV_SIZE_BYTES);
#ifdef __DEBUG__
		printf("aFakeBase color %d\n",aBase->_M_color);
		printf("aFakeBase parent %p\n",aBase->_M_parent);
		printf("aFakeBase left  %p\n",aBase->_M_left);
		printf("aFakeBase right %p\n",aBase->_M_right);
#endif		
		treenode_def* aNode = (treenode_def*) aBase->_M_parent;

#ifdef __DEBUG__
		printf("aFakeBase %s\n",aNode->_M_value_field.first._field->getName().c_str());
#endif

		long aNumOfNodes = *((long*) ((long) &_building_map + ENV_SIZE_BYTES*5)); // 5° long contains number of nodes

#ifdef __DEBUG__
		printf("Number of nodes %d\n",(int) aNumOfNodes);
#endif
		
		void* aPointer;
		void* aFirst = (void*) aBase->_M_left;
		void* aLast = (void*) aBase->_M_right;
		std_map_field::copyNode(aNode, _second, 0, aPointer, aFirst, aLast);

		*((long*) ((long) aAddress+0*ENV_SIZE_BYTES)) = 0; // Struct less
		*((long*) ((long) aAddress+1*ENV_SIZE_BYTES)) = 0; // color
		*((void**) ((long) aAddress+2*ENV_SIZE_BYTES)) = aPointer; // parent
		*((void**) ((long) aAddress+3*ENV_SIZE_BYTES)) = aFirst;
		*((void**) ((long) aAddress+4*ENV_SIZE_BYTES)) = aLast;
		*((long*) ((long) aAddress+5*ENV_SIZE_BYTES)) = aNumOfNodes;

		// finish !!

	}

	void std_map_field::fromJsonMap(const jsonxx::Value& iJson, void* iEntryPoint){
		abort(); // to implement
	}

	void std_map_field::toCpp(std::ostringstream& iStream){
		iStream << _name.c_str() << "[" ;
		_repField->toCpp(iStream);
		iStream << "] ";
	}


}}
