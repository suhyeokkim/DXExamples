#pragma once

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

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
