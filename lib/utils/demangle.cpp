#include "demangle.h"

#include <execinfo.h>
#include <cxxabi.h>
#include <string.h>

extern "C" int demangle(char* oBuffer, int iSize, const char* iMangled){
	
	int status;
	char* aDemangled = abi::__cxa_demangle(iMangled, 0, 0, &status);

	if (status==-2) { // Invalid C++ name
		int dSize = strlen(iMangled);
		dSize = (dSize>iSize?iSize:dSize);
		strncpy(oBuffer,iMangled, dSize);
		oBuffer[dSize]=0;
	} else {
		int dSize = strlen(aDemangled);
		dSize = (dSize>iSize?iSize:dSize);
		strncpy(oBuffer,aDemangled, dSize);
		oBuffer[dSize]=0;
	}

	return 0;			

}

extern "C" int demangleFunctionPointer(char* oBuffer, int iSize, void *func){

	int status;
	char aMangled[2048];

	memset(oBuffer,0,2048);

	char** data = backtrace_symbols(&func, 1);

	char* aFirst = strchr(data[0],'[');
	if ((aFirst!=0) && (aFirst-data[0]==0)) { // If the returned buffer contains only the pointer address
	
		int dSize = strlen(data[0]);
		dSize = (dSize>iSize?iSize:dSize);
		strncpy(oBuffer,data[0], dSize);

		return 1;
	}	

	aFirst = strchr(data[0],'(');
	char* aLast = strchr(data[0],')');

	int aSize = aLast-aFirst-1;
	if ((aSize<iSize) && (aSize<2048) ) {
		
		strncpy ( aMangled, (char*) (aFirst+1), aSize);
		aMangled[aSize]=0;
		aLast = strrchr( aMangled,'+');
		if (aLast!=0) aLast[0] = 0;

		if (strlen(aMangled)==0) return 0;
		char* aDemangled = abi::__cxa_demangle(aMangled, 0, 0, &status);

		if (status==-2) { // Invalid C++ name
			int dSize = strlen(aMangled);
			dSize = (dSize>iSize?iSize:dSize);
			strncpy(oBuffer,aMangled, dSize);
			oBuffer[dSize]=0;
		} else {
			int dSize = strlen(aDemangled);
			dSize = (dSize>iSize?iSize:dSize);
			strncpy(oBuffer,aDemangled, dSize);
			oBuffer[dSize]=0;
		}			

		return 1;	
	}

	return 0;

}
