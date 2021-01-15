#pragma once

#if defined (DEBUG) | (_DEBUG)
	#define _CRTDBG_MAP_ALLOC 
#endif

#include <vector>
#include <cstdlib>
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
	size_t len = wcslen(src); \
	dst = (wchar_t*)alloc(sizeof(wchar_t) * (len + 1)); \
	wcscpy_s(dst, (len + 1), src); \
}

#define ALLOC_AND_STRCPY(dst, src, alloc) \
{ \
	size_t len = strlen(src); \
	dst = (char*)alloc(sizeof(char) * (len + 1)); \
	strcpy_s(dst, (len + 1), src); \
}

#define ALLOC_RANGE_MEMCPY(count_val, new_count, type, dst, src, alloc) \
{ \
	count_val = static_cast<decltype(count_val)>(new_count); \
	dst = (type*)alloc(sizeof(type) * count_val); \
	memcpy(dst, src, sizeof(type) * count_val); \
}

#define ALLOC_RANGE_ZEROMEM(count_val, new_count, type, dst, alloc) \
{ \
	count_val = static_cast<decltype(count_val)>(new_count); \
	dst = (type*)alloc(sizeof(type) * count_val); \
	memset(dst, 0, sizeof(type) * count_val); \
}

#define REALLOC_RANGE_ZEROMEM(start, origin_count, new_count, type, ptr, reallocer) \
	uint start = static_cast<uint>(origin_count); \
	origin_count += static_cast<decltype(origin_count)>(new_count); \
	ptr = (type*)reallocer(ptr, sizeof(type) * origin_count); \
	memset(ptr + start, 0, sizeof(type) * new_count); 

#define REALLOC_EXISTVAL_RANGE_ZEROMEM(start, origin_count, new_count, type, ptr, reallocer) \
	start = static_cast<decltype(start)>(origin_count); \
	origin_count += static_cast<decltype(origin_count)>(new_count); \
	ptr = (type*)reallocer(ptr, sizeof(type) * origin_count); \
	memset(ptr + start, 0, sizeof(type) * new_count); 

#define REALLOC_RANGE_MEMCPY(start, origin_count, new_count, type, dest, src, reallocer) \
	uint start = static_cast<uint>(origin_count); \
	origin_count += static_cast<decltype(origin_count)>(new_count); \
	dest = (type*)reallocer(dest, sizeof(type) * origin_count); \
	memcpy(dest, src, sizeof(type) * new_count); 

#define REALLOC_EXISTVAL_RANGE_MEMCPY(start, origin_count, new_count, type, dest, src, reallocer) \
	start = static_cast<decltype(start)>(origin_count); \
	origin_count += static_cast<decltype(origin_count)>(new_count); \
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
struct PersistantAllocaters : Allocaters
{
	const size_t defaultArenaSize = 16 * 1024 * 1024; // 16MB

	struct Arena
	{
		size_t capacity;
		size_t size;
		byte* ptr;
	};
	size_t arenaSize;
	size_t avaiableIndex;
	std::vector<Arena> arenaVector;

	void* Allocate(size_t s)
	{
		Arena& a = arenaVector[avaiableIndex];
		if (a.capacity < a.size + s)
		{
			AddArena(s);
			avaiableIndex++;
		}

		a.size += s;
		return a.ptr + a.size - s;
	}
	void Deallocate(void* ptr) { }
	void* Reallocate(void* ptr, size_t resized) { return nullptr; }

	void AddArena(size_t capacity = 0)
	{
		size_t cap = arenaSize < capacity ? capacity : arenaSize;
		Arena a = { cap, 0, (byte*)malloc(cap) };
		arenaVector.push_back(a);
	}

	PersistantAllocaters() : arenaSize(defaultArenaSize), avaiableIndex(0), arenaVector()
	{
		AddArena();

		Allocaters::alloc = std::bind(&PersistantAllocaters::Allocate, this, std::placeholders::_1);
		Allocaters::dealloc = std::bind(&PersistantAllocaters::Deallocate, this, std::placeholders::_1);
		Allocaters::realloc = std::bind(&PersistantAllocaters::Reallocate, this, std::placeholders::_1, std::placeholders::_2);
	}
	PersistantAllocaters(size_t arenaSize) : arenaSize(arenaSize), avaiableIndex(0), arenaVector()
	{
		AddArena();

		Allocaters::alloc = std::bind(&PersistantAllocaters::Allocate, this, std::placeholders::_1);
		Allocaters::dealloc = std::bind(&PersistantAllocaters::Deallocate, this, std::placeholders::_1);
		Allocaters::realloc = std::bind(&PersistantAllocaters::Reallocate, this, std::placeholders::_1, std::placeholders::_2);
	}
	~PersistantAllocaters()
	{
		for (size_t i = 0; i < arenaVector.size(); i++)
			SAFE_FREE(arenaVector[i].ptr);
	}
};
struct TempararyAllocaters : Allocaters
{
	const size_t defaultSize = 32 * 1024 * 1024; // 32MB
	struct MemRange { size_t start; size_t count; };

	size_t bufferSize;
	byte* bufferPtr;
	std::vector<MemRange> rangeVector;

	void* Allocate(size_t s)
	{
		if (rangeVector.size())
		{
			size_t startIndex = rangeVector.size() - 1;
			size_t lastDiff = bufferSize - rangeVector[startIndex].start - rangeVector[startIndex].count;

			for (size_t i = 0; i < rangeVector.size() - 1; i++)
			{
				size_t fractional = rangeVector[i + 1].start - rangeVector[i].start - rangeVector[i].count;
				if (s < fractional && fractional - s < lastDiff)
				{
					startIndex = i;
					lastDiff = fractional - s;
				}
			}
				
			if (s > lastDiff)
			{
				/*
				bufferPtr = (byte*)realloc(bufferPtr, bufferSize + s - lastDiff);
				bufferSize = bufferSize + s - lastDiff;
				return bufferPtr + bufferSize;
				*/
				return nullptr;
			}
			else
			{
				MemRange r = { startIndex, s };
				rangeVector.insert(rangeVector.begin() + startIndex, r);
				return bufferPtr + startIndex;
			}
		}
		else
		{
			if (s > bufferSize)
			{
				bufferPtr = (byte*)realloc(bufferPtr, s);
				bufferSize = s;
			}
			MemRange r = { 0, s };
			rangeVector.push_back(r);
			return bufferPtr;
		}	
	}
	void Deallocate(void* ptr) 
	{ 
		for (size_t i = 0; i < rangeVector.size(); i++)
			if (bufferPtr + rangeVector[i].start == ptr)
			{
				rangeVector.erase(rangeVector.begin() + i);
				break;
			}
	}
	void* Reallocate(void* ptr, size_t resized)
	{
		Deallocate(ptr);
		return Allocate(resized);
	}

	TempararyAllocaters() :
		bufferSize(defaultSize), bufferPtr(nullptr), rangeVector()
	{
		bufferPtr = (byte*)malloc(bufferSize);
		memset(bufferPtr, 0, bufferSize);

		Allocaters::alloc = std::bind(&TempararyAllocaters::Allocate, this, std::placeholders::_1);
		Allocaters::dealloc = std::bind(&TempararyAllocaters::Deallocate, this, std::placeholders::_1);
		Allocaters::realloc = std::bind(&TempararyAllocaters::Reallocate, this, std::placeholders::_1, std::placeholders::_2);
	}
	TempararyAllocaters(size_t bufferSize) :
		bufferSize(bufferSize), bufferPtr(nullptr), rangeVector()
	{
		bufferPtr = (byte*)malloc(bufferSize);
		memset(bufferPtr, 0, bufferSize);

		Allocaters::alloc = std::bind(&TempararyAllocaters::Allocate, this, std::placeholders::_1);
		Allocaters::dealloc = std::bind(&TempararyAllocaters::Deallocate, this, std::placeholders::_1);
		Allocaters::realloc = std::bind(&TempararyAllocaters::Reallocate, this, std::placeholders::_1, std::placeholders::_2);
	}
	~TempararyAllocaters()
	{
		SAFE_FREE(bufferPtr);
	}
};
struct OSAllocater : Allocaters
{
	OSAllocater() 
	{
		Allocaters::alloc = malloc;
		Allocaters::dealloc = free;
		Allocaters::realloc = realloc;
	}
};
struct GeneralAllocater
{
	/*PersistantAllocaters*/OSAllocater persistant;
	TempararyAllocaters temparary;
	OSAllocater system;

	GeneralAllocater() : persistant(), temparary(), system() { }
};

struct ProfileTime
{
	const char* string;
	DWORD startTime;

	ProfileTime(const char* string) : string(string)
	{
		startTime = GetTickCount();
	}
	~ProfileTime()
	{
		printf("%s : %dms\n", string, GetTickCount() - startTime);
	}
};
