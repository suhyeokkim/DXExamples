#pragma once

#include "symbols.h"
#include "defined_type.h"

#include <string.h>

#define ALLOCATOR_MIN_ALIGNMENT 8

#define SYSTEM_NAME "system"
#define PERSISTANT_NAME "persitant"
#define TEMPARARY_NAME "temp"

DECLSPEC_DLL void* memAlloc(size_t size, size_t alignment, size_t alignOffset, const char* addrspace = SYSTEM_NAME);
DECLSPEC_DLL bool memFree(void* ptr, const char* addrspace = SYSTEM_NAME);
DECLSPEC_DLL size_t memAllocSize(const char* addrspace);
DECLSPEC_DLL size_t memPageSize(const char* addrspace);
DECLSPEC_DLL size_t memMinPageSize();
DECLSPEC_DLL int32 validPageCount();
