#pragma once

#include "symbols.h"
#include "defined_type.h"

#include <string.h>

#define SYSTEM_NAME "system"
#define PERSISTANT_NAME "persitant"
#define TEMPARARY_NAME "temp"

DECLSPEC_DLL void* memAlloc(size_t size, size_t alignment, size_t alignOffset, const char* addrspace = SYSTEM_NAME);
DECLSPEC_DLL bool memFree(void* ptr, const char* addrspace = SYSTEM_NAME);
DECLSPEC_DLL size_t memAllocSize(const char* addrspace);
DECLSPEC_DLL size_t memPageSize(const char* addrspace);

// EASTL 레거시 코드
DECLSPEC_DLL void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
DECLSPEC_DLL void* operator new[](size_t size, const char* pName, const char* file, int line);
DECLSPEC_DLL void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
DECLSPEC_DLL void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, const char* file, int line);
DECLSPEC_DLL void operator delete[](void* p, const char* pName, const char* file, int line);
DECLSPEC_DLL void operator delete[](void* p, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
DECLSPEC_DLL void operator delete[](void* p, unsigned align, const char* pName, const char* file, int line);
DECLSPEC_DLL void operator delete[](void* p, unsigned align, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
