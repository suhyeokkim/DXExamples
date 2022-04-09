#pragma once

#pragma region 메모리 해제

#define SAFE_FREE(x) \
    if (x) { \
        free(x); \
    }

#define SAFE_ALIGNED_FREE(x) \
    if (x) { \
        _aligned_free(x); \
    }

#define SAFE_RELEASE(x) \
    if (x) { \
        x->Release(); \
    }

#pragma endregion

#pragma region 에러 핸들링

#define LOG(str) \
    { \
        auto value = str; \
        _putws(value); \
        OutputDebugStringW(value); \
    }

#define LOG_ARGS(fmt, ...) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, fmt, __VA_ARGS__); \
        _putws(szErrorBuffer); \
        OutputDebugStringW(szErrorBuffer); \
    }

#define ASSERT(x) assert(x);
#define ERROR_MESSAGE(str) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
    }
#define ERROR_MESSAGE_ARGS(fmt, ...) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
        LOG(szErrorBuffer2); \
    }
#define ERROR_MESSAGE_CONTINUE(str) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        continue; \
    }
#define ERROR_MESSAGE_CONTINUE_ARGS(fmt, ...) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
        LOG(szErrorBuffer2); \
        continue; \
    }
#define ERROR_MESSAGE_GOTO(str, go) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        goto go; \
    }
#define ERROR_MESSAGE_RETURN(str) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return -1; \
    }
#define ERROR_MESSAGE_RETURN_CODE(str, failCode) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return failCode; \
    }
#define ERROR_MESSAGE_RETURN_VOID(str) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return; \
    }
#define WARN_MESSAGE(str) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
    }
#define WARN_MESSAGE_ARGS(fmt, ...) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"(WARN) MESSAGE::%s\n", szErrorBuffer1); \
        LOG(szErrorBuffer2); \
    }
#define WARN_MESSAGE_CONTINUE(str) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        continue; \
    }
#define WARN_MESSAGE_GOTO(str, go) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        goto go; \
    }
#define WARN_MESSAGE_RETURN(str) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return -1; \
    }
#define WARN_MESSAGE_RETURN_CODE(str, failCode) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return failCode; \
    }
#define WARN_MESSAGE_RETURN_VOID(str) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return; \
    }
#define FALSE_ERROR_MESSAGE(x, str) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
    }
#define FALSE_ERROR_MESSAGE_ARGS(x, fmt, ...) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
        LOG(szErrorBuffer2); \
    }
#define FALSE_ERROR_MESSAGE_CONTINUE(x, str) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        continue; \
    }
#define FALSE_ERROR_MESSAGE_CONTINUE_ARGS(x, fmt, ...) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
        LOG(szErrorBuffer2); \
        continue; \
    }
#define FALSE_ERROR_MESSAGE_GOTO(x, str, go) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        goto go; \
    }
#define FALSE_ERROR_MESSAGE_RETURN(x, str) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return -1; \
    }
#define FALSE_ERROR_MESSAGE_THROW(x, str, exp) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        throw exp(); \
    }
#define FALSE_ERROR_MESSAGE_RETURN_CODE(x, str, failCode) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return failCode; \
    }
#define FALSE_ERROR_MESSAGE_ARGS_RETURN_CODE(x, fmt, failCode, ...) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"*ERROR* MESSAGE::%s\n", szErrorBuffer1); \
        LOG(szErrorBuffer2); \
        return failCode; \
    }
#define FALSE_ERROR_MESSAGE_RETURN_VOID(x, str) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return; \
    }
#define FALSE_WARN_MESSAGE(x, str) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
    }
#define FALSE_WARN_MESSAGE_ARGS(x, fmt, ...) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"(WARN) MESSAGE::%s\n", szErrorBuffer1); \
        LOG(szErrorBuffer2); \
    }
#define FALSE_WARN_MESSAGE_CONTINUE(x, str) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        continue; \
    }
#define FALSE_WARN_MESSAGE_GOTO(x, str, go) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        goto go; \
    }
#define FALSE_WARN_MESSAGE_RETURN(x, str) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return -1; \
    }
#define FALSE_WARN_MESSAGE_RETURN_CODE(x, str, failCode) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
        return failCode; \
    }
#define FALSE_WARN_MESSAGE_RETURN_VOID(x, str) \
    if (!(x)) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) MESSAGE::%s\n", str); \
        LOG(szErrorBuffer); \
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
        wsprintfW(szErrorBuffer, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
    }
#define FAILED_ERROR_MESSAGE_ARGS(x, fmt, ...) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer2); \
    }
#define FAILED_ERROR_MESSAGE_CONTINUE(x, str) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        continue; \
    }
#define FAILED_ERROR_MESSAGE_CONTINUE_ARGS(x, fmt, ...) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer2); \
    }
#define FAILED_ERROR_MESSAGE_GOTO(x, str, go) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        goto go; \
    }
#define FAILED_ERROR_MESSAGE_RETURN(x, str) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        return (HRESULT)(x); \
    }
#define FAILED_ERROR_MESSAGE_THROW(x, str, exp) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        throw exp(); \
    }
#define FAILED_ERROR_MESSAGE_RETURN_ARGS(x, fmt, ...) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer2); \
        return (HRESULT)(x); \
    }
#define FAILED_ERROR_MESSAGE_RETURN_CODE(x, str, failCode) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        return failCode; \
    }
#define FAILED_ERROR_MESSAGE_RETURN_VOID(x, str) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"*ERROR* CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        return; \
    }
#define FAILED_WARN_MESSAGE(x, str) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
    }
#define FAILED_WARN_MESSAGE_ARGS(x, fmt, ...) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"(WARN) CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer2); \
    }
#define FAILED_WARN_MESSAGE_CONTINUE(x, str) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        continue; \
    }
#define FAILED_WARN_MESSAGE_CONTINUE_ARGS(x, fmt, ...) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer1[MAX_PATH]; \
        wsprintfW(szErrorBuffer1, fmt, __VA_ARGS__); \
        WCHAR szErrorBuffer2[MAX_PATH]; \
        wsprintfW(szErrorBuffer2, L"(WARN) CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer2); \
        continue; \
    }
#define FAILED_WARN_MESSAGE_GOTO(x, str, go) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        goto go; \
    }
#define FAILED_WARN_MESSAGE_RETURN(x, str) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        return (HRESULT)(x); \
    }
#define FAILED_WARN_MESSAGE_RETURN_CODE(x, str, failCode) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        return failCode; \
    }
#define FAILED_WARN_MESSAGE_RETURN_VOID(x, str) \
    if (((HRESULT)(x)) < 0) \
    { \
        WCHAR szErrorBuffer[MAX_PATH]; \
        wsprintfW(szErrorBuffer, L"(WARN) CODE::0x%x, MESSAGE::%s\n", x, str); \
        LOG(szErrorBuffer); \
        return; \
    }
#define FAILED_RETURN(hr)  \
    if (FAILED(hr)) \
    { \
        return hr; \
    }
#define FAILED_RETURN_VOID(hr)  \
    if (FAILED(hr)) \
    { \
        return; \
    }

#pragma endregion

#pragma region 디버그 이름 설정

#define SET_DX_DEBUG_NAME(dxItem, wcs) (dxItem)->SetPrivateData(WKPDID_D3DDebugObjectNameW, sizeof(wcs), wcs)

#pragma endregion