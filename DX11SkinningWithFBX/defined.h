#pragma once

#if defined (DEBUG) | (_DEBUG)
	#define _CRTDBG_MAP_ALLOC 
#endif

#include <cassert>
#include <windows.h>
#include <functional>

#define IN
#define OUT
#define INOUT
#define REF

#define ASSERT(x) assert(x);
#define ERROR_MESSAGE(str) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
	}
#define ERROR_MESSAGE_ARGS(fmt, ...) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
	}
#define ERROR_MESSAGE_CONTINUE(str) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		continue; \
	}
#define ERROR_MESSAGE_CONTINUE_ARGS(fmt, ...) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
		continue; \
	}
#define ERROR_MESSAGE_GOTO(str, go) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		goto go; \
	}
#define ERROR_MESSAGE_RETURN(str) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return -1; \
	}
#define ERROR_MESSAGE_RETURN_CODE(str, failcode) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return failcode; \
	}
#define ERROR_MESSAGE_RETURN_VOID(str) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return; \
	}
#define WARN_MESSAGE(str) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
	}
#define WARN_MESSAGE_ARGS(fmt, ...) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"(WARN) MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
	}
#define WARN_MESSAGE_CONTINUE(str) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		continue; \
	}
#define WARN_MESSAGE_GOTO(str, go) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		goto go; \
	}
#define WARN_MESSAGE_RETURN(str) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return -1; \
	}
#define WARN_MESSAGE_RETURN_CODE(str, failcode) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return failcode; \
	}
#define WARN_MESSAGE_RETURN_VOID(str) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return; \
	}
#define FALSE_ERROR_MESSAGE(x, str) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
	}
#define FALSE_ERROR_MESSAGE_ARGS(x, fmt, ...) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
	}
#define FALSE_ERROR_MESSAGE_CONTINUE(x, str) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		continue; \
	}
#define FALSE_ERROR_MESSAGE_CONTINUE_ARGS(x, fmt, ...) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
		continue; \
	}
#define FALSE_ERROR_MESSAGE_GOTO(x, str, go) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		goto go; \
	}
#define FALSE_ERROR_MESSAGE_RETURN(x, str) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return -1; \
	}
#define FALSE_ERROR_MESSAGE_RETURN_CODE(x, str, failcode) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return failcode; \
	}
#define FALSE_ERROR_MESSAGE_ARGS_RETURN_CODE(x, fmt, failcode, ...) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
		return failcode; \
	}
#define FALSE_ERROR_MESSAGE_RETURN_VOID(x, str) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return; \
	}
#define FALSE_WARN_MESSAGE(x, str) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
	}
#define FALSE_WARN_MESSAGE_ARGS(x, fmt, ...) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"(WARN) MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
	}
#define FALSE_WARN_MESSAGE_CONTINUE(x, str) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		continue; \
	}
#define FALSE_WARN_MESSAGE_GOTO(x, str, go) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		goto go; \
	}
#define FALSE_WARN_MESSAGE_RETURN(x, str) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return -1; \
	}
#define FALSE_WARN_MESSAGE_RETURN_CODE(x, str, failcode) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return failcode; \
	}
#define FALSE_WARN_MESSAGE_RETURN_VOID(x, str) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return; \
	}
#define FALSE_RETURN(x)  \
	if (!x) \
	{ \
		return hr; \
	}
#define FAILED_ERROR_MESSAGE(x, str) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
	}
#define FAILED_ERROR_MESSAGE_ARGS(x, fmt, ...) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"*ERROR* CODE:%x, MESSAGE::%s\n", x, szErrorBuffer1); \
		_putws(szErrorBuffer2); \
	}
#define FAILED_ERROR_MESSAGE_CONTINUE(x, str) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		continue; \
	}
#define FAILED_ERROR_MESSAGE_CONTINUE_ARGS(x, fmt, ...) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
	}
#define FAILED_ERROR_MESSAGE_GOTO(x, str, go) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		goto go; \
	}
#define FAILED_ERROR_MESSAGE_RETURN(x, str) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return (HRESULT)(x); \
	}
#define FAILED_ERROR_MESSAGE_RETURN_ARGS(x, fmt, ...) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
		return (HRESULT)(x); \
	}
#define FAILED_ERROR_MESSAGE_RETURN_CODE(x, str, failcode) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return failcode; \
	}
#define FAILED_ERROR_MESSAGE_RETURN_VOID(x, str) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return; \
	}
#define FAILED_WARN_MESSAGE(x, str) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
	}
#define FAILED_WARN_MESSAGE_ARGS(x, fmt, ...) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"(WARN) MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
	}
#define FAILED_WARN_MESSAGE_CONTINUE(x, str) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		continue; \
	}
#define FAILED_WARN_MESSAGE_CONTINUE_ARGS(x, fmt, ...) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer1[MAX_PATH]; \
		wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
		WCHAR szErrorBuffer2[MAX_PATH]; \
		wsprintfW(szErrorBuffer2, L"(WARN) MESSAGE::%s\n", szErrorBuffer1); \
		_putws(szErrorBuffer2); \
		continue; \
	}
#define FAILED_WARN_MESSAGE_GOTO(x, str, go) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		goto go; \
	}
#define FAILED_WARN_MESSAGE_RETURN(x, str) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return (HRESULT)(x); \
	}
#define FAILED_WARN_MESSAGE_RETURN_CODE(x, str, failcode) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return failcode; \
	}
#define FAILED_WARN_MESSAGE_RETURN_VOID(x, str) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
		_putws(szErrorBuffer); \
		return; \
	}
#define FAILED_RETURN(hr)  \
	if (FAILED(hr)) \
	{ \
		return hr; \
	}
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

#define ALLOC_AND_WCSCPY(dst, src, alloc) \
{ \
	uint len = wcslen(src); \
	dst = (wchar_t*)alloc(sizeof(wchar_t) * (len + 1)); \
	wcscpy_s(dst, (len + 1), src); \
}

#define ALLOC_AND_STRCPY(dst, src, alloc) \
{ \
	uint len = strlen(src); \
	dst = (char*)alloc(sizeof(char) * (len + 1)); \
	strcpy_s(dst, (len + 1), src); \
}

#define REALLOC_RANGE_ZEROMEM(start, origin_count, new_count, type, ptr, reallocer) \
	uint start = (uint)origin_count; \
	origin_count += (uint)new_count; \
	ptr = (type*)reallocer(ptr, sizeof(type) * origin_count); \
	memset(ptr + start, 0, sizeof(type) * new_count); 

#define REALLOC_EXISTVAL_RANGE_ZEROMEM(start, origin_count, new_count, type, ptr, reallocer) \
	start = (uint)origin_count; \
	origin_count += (uint)new_count; \
	ptr = (type*)reallocer(ptr, sizeof(type) * origin_count); \
	memset(ptr + start, 0, sizeof(type) * new_count); 

#define REALLOC_RANGE_MEMCPY(start, origin_count, new_count, type, dest, src, reallocer) \
	uint start = (uint)origin_count; \
	origin_count += (uint)new_count; \
	dest = (type*)reallocer(dest, sizeof(type) * origin_count); \
	memcpy(dest, src, sizeof(type) * new_count); 

#define REALLOC_EXISTVAL_RANGE_MEMCPY(start, origin_count, new_count, type, dest, src, reallocer) \
	start = (uint)origin_count; \
	origin_count += (uint)new_count; \
	dest = (type*)reallocer(dest, sizeof(type) * origin_count); \
	memcpy(dest, src, sizeof(type) * new_count); 

typedef unsigned short ushort;
typedef signed char sbyte;
typedef unsigned char ubyte;
typedef signed int sint;
typedef unsigned int uint;
typedef signed long slong;
typedef unsigned long ulong;
typedef long long int lint;
typedef signed long long int slint;
typedef unsigned long long int ulint;

struct Allocaters
{
	std::function<void*(size_t)> alloc;
	std::function<void*(void*, size_t)> realloc;
	std::function<void(void*)> dealloc;
};