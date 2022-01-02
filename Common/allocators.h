#pragma once

#include "symbols.h"
#include "defined_type.h"

#include <string.h>

#define SYSTEM_NAME L"system"
#define PERSISTANT_NAME L"persitant"
#define TEMPARARY_NAME L"temp"

DECLSPEC_DLL void* memAlloc(size_t size, size_t alignment, size_t alignOffset, const wchar_t* addrspace = SYSTEM_NAME);
DECLSPEC_DLL bool memFree(void* ptr, const wchar_t* addrspace = SYSTEM_NAME);
DECLSPEC_DLL size_t memAllocSize(const wchar_t* addrspace);
DECLSPEC_DLL size_t memPageSize(const wchar_t* addrspace);
DECLSPEC_DLL size_t memPageMinSize(bool pageLocked);
DECLSPEC_DLL int32 validPageCount();
DECLSPEC_DLL bool memPageAdd(const wchar_t* addrspace, size_t pageSize, bool pageLocked);
DECLSPEC_DLL bool memPageFree(const wchar_t* addrspace);