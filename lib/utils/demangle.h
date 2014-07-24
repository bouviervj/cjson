#ifndef _DEMANGLE_H_
#define _DEMANGLE_H_

extern "C" int demangle(char* oBuffer, int iSize, const char* iMangled);
extern "C" int demangleFunctionPointer(char* oBuffer, int iSize, void *func);

#endif
