#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <fcntl.h> 
#include <cstdio>

#include "winmain.h"
#include "dxentry.h"
#include "defined_macro.h"

using namespace std;

struct RedirectConsoleScope
{
    RedirectConsoleScope()
    {
        AllocConsole();

        FILE* stream;
        freopen_s(&stream, "CONIN$", "r", stdin);
        freopen_s(&stream, "CONOUT$", "w", stdout);
        freopen_s(&stream, "CONOUT$", "w", stderr);

        puts("[RedirectConsoleScope] ok");
    }

    ~RedirectConsoleScope()
    {
        FreeConsole();

        puts("[RedirectConsoleScope] off");
    }
};

struct WindowScope
{
    WindowInstance wndInst;

    HRESULT Init(HINSTANCE hInstance)
    {
        DebugPrintScope _(L"WindowScope::Init");

        // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
        // Using this awareness context allows the client area of the window 
        // to achieve 100% scaling while still allowing non-client window content to 
        // be rendered in a DPI sensitive fashion.
        SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        // Register the windows class
        wndInst.wndClass = GetWindowClass(hInstance, MsgProc, L"Direct3DWindowClass");
        FALSE_ERROR_MESSAGE_RETURN_CODE(RegisterClassW(&wndInst.wndClass), L"fail to register window class..", E_FAIL);

        GetDXWindowSetting(&wndInst.settings);
        const WindowSetting& wndSet = wndInst.settings;
        wndInst.hWnd = GetCreatedWindow(hInstance, &wndInst.wndClass, wndSet.windowName, wndSet.windowStyle, wndSet.windowWidth, wndSet.windowHeight);

        ShowWindow(wndInst.hWnd, SW_SHOW);
        UpdateWindow(wndInst.hWnd);

        FAILED_ERROR_MESSAGE_RETURN(
            DXEntryInit(&wndInst, hInstance, wndInst.hWnd, wndSet.windowWidth, wndSet.windowHeight, wndSet.maxFrameRate, true),
            L"fail to initialize."
        );

        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::GetWindowRect(wndInst.hWnd, &wndInst.rect), L"failed to GetWindowRect..", E_FAIL
        );

        return S_OK;
    }

    void Loop()
    {
        DebugPrintScope _(L"WindowScope::Loop");

        MSG msg = { 0, };

        while (msg.message != WM_QUIT)
        {
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
            else
            {
                DXEntryFrameUpdate(&wndInst);
            }
        }
    }

    ~WindowScope()
    {
        DebugPrintScope _(L"~WindowScope");

        DXEntryClean(&wndInst);
    }

    static LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        //DebugPrintScope _(L"WindowScope::MsgProc");

        PAINTSTRUCT ps;
        HDC hdc;

        switch (uMsg)
        {
        case WM_PAINT:
            hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_SIZE:
        {
            RECT clientRect = {};
            ::GetClientRect(hWnd, &clientRect);

            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;

            DXEntryReserveResize(width, height);
        }
            break;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    static WNDCLASSW GetWindowClass(HINSTANCE hInstance, WNDPROC msgProc, LPCWSTR className)
    {
        DebugPrintScope _(L"WindowScope::GetWindowClass");

        WNDCLASSW wndClass = {};
        wndClass.style = CS_DBLCLKS;
        wndClass.lpfnWndProc = msgProc;
        wndClass.hInstance = hInstance;
        wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndClass.lpszClassName = className;
        return wndClass;
    }

    static HWND GetCreatedWindow(HINSTANCE hInstance, LPWNDCLASSW wndClass, LPCWSTR windowTitle, DWORD windowStyle, UINT width, UINT height)
    {
        DebugPrintScope _(L"WindowScope::GetCreatedWindow");

        RECT rc;
        SetRect(&rc, 0, 0, width, height);
        AdjustWindowRect(&rc, windowStyle, false);

        return CreateWindow(wndClass->lpszClassName, windowTitle, windowStyle, 100, 100, width, height, 0, (HMENU)nullptr, hInstance, 0);
    }
};

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    RedirectConsoleScope _;
    WindowScope wndScope;

    FAILED_ERROR_MESSAGE_RETURN(
        wndScope.Init(hInstance), 
        L"WindowScope init fail.."
    );
    wndScope.Loop();

#if defined(_DEBUG)
    puts("[DEBUG] show logs");
    auto var = getchar();
#endif

    return 0;
}