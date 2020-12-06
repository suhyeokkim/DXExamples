#include <windows.h>

#pragma once

#define IN
#define OUT
#define REF

#define FALSE_MESSAGE_RETURN(x, str) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		MessageBoxW(GetForegroundWindow(), szErrorBuffer, L"Unexpected error encountered", MB_OK | MB_ICONERROR); \
		return -1; \
	}
#define FALSE_MESSAGE_RETURN_CODE(x, str, failcode) \
	if (!(x)) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		MessageBoxW(GetForegroundWindow(), szErrorBuffer, L"Unexpected error encountered", MB_OK | MB_ICONERROR); \
		return failcode; \
	}
#define FAILED_MESSAGE_RETURN(x, str) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		MessageBoxW(GetForegroundWindow(), szErrorBuffer, L"Unexpected error encountered", MB_OK | MB_ICONERROR); \
		return (HRESULT)(x); \
	}
#define FAILED_MESSAGE_RETURN_CODE(x, str, failcode) \
	if (((HRESULT)(x)) < 0) \
	{ \
		WCHAR szErrorBuffer[MAX_PATH]; \
		wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
		MessageBoxW(GetForegroundWindow(), szErrorBuffer, L"Unexpected error encountered", MB_OK | MB_ICONERROR); \
		return failcode; \
	}
#define FAILED_RETURN(hr)  \
	if (FAILED(hr)) \
	{ \
		return hr; \
	}