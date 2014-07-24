Introduction
============
This project aims at serializing C/C++ classes or structures without inclusive code in your C/C++ definitions. It uses GCCXML outputs to build a dictionary describing internal structures. This library is an experimental project.

Example
=======

Defining these classes: 

```C

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


```

You can serialize/deserialize class1 object like that :

```C

        // Random class initialization
        com::class1* aObject = new com::class1();

        for (int i=0; i<10; i++){
                aObject->setData(i,i);
        }      

        aObject->pdata = new char[7];
        for (int i=0; i<7; i++){
                aObject->pdata[i] = 7-i;
        }
       
```

Serialization :

```C
        // dictionary initialization
        cjson::dictionary aDict("./data/dictionary.xml");

        // json transformation
        std::string aJson = aDict.toJson<com::class1>(aObject);

        // print encoded class
        cout << aJson << std::endl ;

```

Unserialization :
```C
        // decode the object
        com::class1* aDecodedObject = aDict.fromJson<com::class1>(aJson);

        // modify data
        aDecodedObject->setData(4,22);

        // json transformation
        aJson = aDict.toJson<com::class1>(aDecodedObject);
       
        // print encoded class
        cout << aJson << std::endl ;

```

The printed result is :
```shell
>:~/cjson$ ./main
{"_index":54,"_inner":{"_ident":"test","pi":3.141593},"_name":"first","com::class0::_type":"type","com::class0::data":[0,1,2,3,4,5,6,7,8,9],"com::classb::_ref":"ref","com::classm1::_type":"typem1","com::classm1::pdata":[7,6,5,4,3,2,1]}
{"_index":54,"_inner":{"_ident":"test","pi":3.141593},"_name":"first","com::class0::_type":"type","com::class0::data":[0,1,2,3,22,5,6,7,8,9],"com::classb::_ref":"ref","com::classm1::_type":"typem1","com::classm1::pdata":[7,6,5,4,3,2,1]}
>:~/cjson$ 
```

Features
========

Serialize/Unserialize any class that have been previously compiled with GCCXML:
 * Fundamental types, classes , structs
 * STL supported for std::string, std::vector, std::map 

Constraints
===========

Current limitations of this project: 
 * x86 64 bits (for the moment)
 * GNU Linux
 * light support on virtual bases
 * GCC Itanium ABI version dependency

This project redefines malloc, free, calloc, delete, such can be not convenient with projects redefining themselves these methods.

Dependencies
============

Dependencies to external libs :
 * Xerces : to build internal dictionary
 * ~~Boost~~ : regular expressions, smart pointers
 * Trex : regular expressions
 * Jsonxx : convenient lib to parse Json data


