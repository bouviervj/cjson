#ifndef _CJSON_REGEXP_H_
#define _CJSON_REGEXP_H_

#include <string.h>
#include "./utils/trex/TRexpp.h"

namespace cjson {
	
	class regexp {

		private:

			TRexpp *cregexp; 
			std::string _regexp;
	
		public: 

			regexp(const std::string& iStrRegExp){

				//printf("Trying to compile %s\n", iStrRegExp.c_str());
				cregexp = new TRexpp();
				cregexp->Compile(iStrRegExp.c_str());
				_regexp = iStrRegExp;

			}

			bool matches(const std::string& iStr){

				//printf("Trying matching %s with %s\n", iStr.c_str(), _regexp.c_str());
				
				if (cregexp->Match(iStr.c_str())) {
				//	printf("Success\n");
				 	return true;
				} 

				//printf("Fails\n");
				return false;

			}

			std::string get(int iIndex){

				const char* aStrChar;
				int aLength;
				if (cregexp->GetSubExp(iIndex,&aStrChar,&aLength)) {
					return std::string(aStrChar,aLength);
				} 
				return std::string("");

			}

			

	}; 


}



#endif
