#pragma once

#include "defined_type.h"

#include <string.h>
#include <EASTL/vector.h>
#include <EASTL/array.h>

void* SystemAlloc(size_t size, unsigned debugFlags, const char* file, int line);
void* SystemAlignedAlloc(size_t size, size_t alignment, size_t alignmentOffset, unsigned debugFlags, const char* file, int line);
void SystemDealloc(void* p, unsigned debugFlags, const char* file, int line);
void SystemAlignedDealloc(void* p, unsigned debugFlags, const char* file, int line);

#define EASTL_SYSTEM_NAME ""
#define EASTL_PERSISTANT_NAME "persitant"
#define EASTL_BIG_PERSISTANT_NAME "persitant"
#define EASTL_TEMPARARY_NAME "temp"
#define EASTL_BIG_TEMPARARY_NAME "big_temp"

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
void* operator new[](size_t size, const char* pName, const char* file, int line);
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
void* operator new[](size_t size, size_t alignment, size_t alignmentOffset, const char* pName, const char* file, int line);
void operator delete[](void* p, const char* pName, const char* file, int line);
void operator delete[](void* p, const char* pName, int flags, unsigned debugFlags, const char* file, int line);
void operator delete[](void* p, unsigned align, const char* pName, const char* file, int line);
void operator delete[](void* p, unsigned align, const char* pName, int flags, unsigned debugFlags, const char* file, int line);

class EASTLAllocator
{
public:
	EASTLAllocator(const char* name = nullptr) : name(nullptr)
	{
		if (name)
		{
			size_t len = strlen(name);
			this->name = (char*)::new(nullptr, __FILE__, __LINE__) char[sizeof(char) * (len + 1)];
			strcpy_s(this->name, len + 1, name);
		}
	}
	EASTLAllocator(const EASTLAllocator& o) : name(nullptr)
	{
		if (o.name)
		{
			size_t len = strlen(o.name);
			name = (char*)::new(nullptr, __FILE__, __LINE__) char[sizeof(char) * (len + 1)];
			strcpy_s(name, len + 1, o.name);
		}
	}
	EASTLAllocator(EASTLAllocator&& o) : name(o.name) { }

	EASTLAllocator& operator=(const EASTLAllocator& o)
	{
		size_t len = strlen(o.name);
		name = (char*)::new(nullptr, __FILE__, __LINE__) char[sizeof(char) * (len + 1)];
		strcpy_s(name, len + 1, o.name);
		return *this;
	}
	EASTLAllocator& operator=(EASTLAllocator&& o)
	{
		name = o.name;
		return *this;
	}

	void* allocate(size_t num_bytes, int flags = 0)
	{
		void* p = ::new(name, __FILE__, __LINE__) char[num_bytes];
		return p;
	}

	void* allocate(size_t num_bytes, size_t alignment, size_t offset, int flags = 0)
	{
		void* p = ::new(alignment, offset, name, __FILE__, __LINE__) char[num_bytes];
		return p;
	}

	void deallocate(void* p, size_t num_bytes)
	{
		::operator delete[] (p, name, __FILE__, __LINE__);
	}

	const char* get_name() const { return name; }
	void set_name(char* n) { name = n; }

protected:
	char* name;

};

bool operator==(const EASTLAllocator& a, const EASTLAllocator& b);
bool operator!=(const EASTLAllocator& a, const EASTLAllocator& b);
