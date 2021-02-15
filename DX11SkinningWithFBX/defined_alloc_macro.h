#include "defined_type.h"

#pragma once

#define SAFE_FREE(x) \
	if (x) \
	{ \
		free(x); \
		x = nullptr; \
	}
#define SAFE_DELETE(x) \
	if (x) \
	{ \
		delete (x); \
		x = nullptr; \
	} 
#define SAFE_DELETE_OVERLOADED(x, alloc_name) \
	if (x) \
	{ \
		::operator delete[] (x, alloc_name, __FILE__, __LINE__); \
		x = nullptr; \
	} 
#define SAFE_DEALLOC(x, func) \
	if ((x)) \
	{ \
		func((x)); \
		x = nullptr; \
	} 
#define SAFE_RELEASE(x) \
	if ((x)) \
	{ \
		(x)->Release(); \
		x = nullptr; \
	} 

#define ALLOC_OVERLOADED_VOID_SIZED(alloc_name, dst, size) \
{ \
	dst = (void*)::new(alloc_name, __FILE__, __LINE__) char[size]; \
}
#define ALLOC_OVERLOADED_SIZED(alloc_name, dst, type, size) \
{ \
	dst = (type*)::new(alloc_name, __FILE__, __LINE__) char[size]; \
}
#define ALLOC_OVERLOADED(alloc_name, dst, type, count) \
{ \
	dst = (type*)::new(alloc_name, __FILE__, __LINE__) type[count]; \
}
#define ALLOC_SIZE_AND_WCSCPY(alloc_name, dst, size, src) \
{ \
	size_t len = wcslen(src); \
	dst = (wchar_t*)::new(alloc_name, __FILE__, __LINE__) char[len * sizeof(wchar_t)]; \
	wcscpy_s(dst, (size/sizeof(wchar_t) + 1), src); \
}
#define ALLOC_SIZE_AND_STRCPY(alloc_name, dst, size, src) \
{ \
	size_t len = strlen(src); \
	dst = (char*)::new(alloc_name, __FILE__, __LINE__) char[size]; \
	strcpy_s(dst, (size/sizeof(char) + 1), src); \
}

#define ALLOC_AND_WCSCPY(alloc_name, dst, src) \
{ \
	size_t len = wcslen(src); \
	dst = (wchar_t*)::new(alloc_name, __FILE__, __LINE__) wchar_t[(len + 1)]; \
	wcscpy_s(dst, (len + 1), src); \
}

#define ALLOC_AND_STRCPY(alloc_name, dst, src) \
{ \
	size_t len = strlen(src); \
	dst = (char*)::new(alloc_name, __FILE__, __LINE__) char[(len + 1)]; \
	strcpy_s(dst, (len + 1), src); \
}

#define ALLOC_RANGE_MEMCPY(alloc_name, count_val, new_count, type, dst, src) \
{ \
	count_val = static_cast<decltype(count_val)>(new_count); \
	dst = (type*)::new(alloc_name, __FILE__, __LINE__) type[count_val]; \
	memcpy(dst, src, sizeof(type) * count_val); \
}

#define ALLOC_RANGE_ONLY_MEMCPY(alloc_name, count_val, type, dst, src) \
{ \
	dst = (type*)::new(alloc_name, __FILE__, __LINE__) type[count_val]; \
	memcpy(dst, src, sizeof(type) * count_val); \
}

#define ALLOC_RANGE_ZEROMEM(alloc_name, count_val, new_count, type, dst) \
{ \
	count_val = static_cast<decltype(count_val)>(new_count); \
	dst = (type*)::new(alloc_name, __FILE__, __LINE__) type[count_val]; \
	memset(dst, 0, sizeof(type) * count_val); \
}

#define ALLOC_RANGE_ONLY_ZEROMEM(alloc_name, count_val, type, dst) \
{ \
	dst = (type*)::new(alloc_name, __FILE__, __LINE__) type[count_val]; \
	memset(dst, 0, sizeof(type) * count_val); \
}

#define ALLOC_RANGE_MEMCPY_OTHERZERO(alloc_name, count_val, capacity_val, type, dst, src) \
{ \
	dst = (type*)::new(alloc_name, __FILE__, __LINE__) type[capacity_val]; \
	memcpy(dst, src, sizeof(type) * count_val); \
	memset(dst + count_val, 0, sizeof(type) * (capacity_val - count_val));\
}
