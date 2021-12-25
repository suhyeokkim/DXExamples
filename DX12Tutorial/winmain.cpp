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
    WNDCLASSW wndClass;
    WindowInstance wndInst;
    HWND hWnd;

    HRESULT Init(HINSTANCE hInstance)
    {
        DebugPrintScope _(L"WindowScope::Init");

        // Register the windows class
        WNDCLASSW wndClass = GetWindowClass(hInstance, MsgProc, L"Direct3DWindowClass");
        FALSE_ERROR_MESSAGE_RETURN_CODE(RegisterClassW(&wndClass), L"fail to register window class..", E_FAIL);

        WindowInstance wndInst;
        GetDXWindowSetting(&wndInst);
        const WindowSetting& wndSet = wndInst.settings;
        HWND hWnd = GetCreatedWindow(hInstance, &wndClass, wndSet.windowName, wndSet.windowWidth, wndSet.windowHeight);

        ShowWindow(hWnd, SW_SHOW);
        UpdateWindow(hWnd);

        FAILED_ERROR_MESSAGE_RETURN(
            DXEntryInit(&wndInst.dx, hInstance, hWnd, wndSet.windowWidth, wndSet.windowHeight, wndSet.maxFrameRate, true),
            L"fail to initialize."
        );

        puts("[WindowScope] ok");
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
                DXEntryFrameUpdate(&wndInst.dx);
            }
        }
    }

    ~WindowScope()
    {
        DebugPrintScope _(L"~WindowScope");

        DXEntryClean(&wndInst.dx);

        puts("[WindowScope] off");
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

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        }

        return 0;
    }

    static WNDCLASSW GetWindowClass(HINSTANCE hInstance, WNDPROC msgProc, LPCWSTR className)
    {
        DebugPrintScope _(L"WindowScope::GetWindowClass");

        WNDCLASSW wndClass;
        memset(&wndClass, 0, sizeof(WNDCLASS));
        wndClass.style = CS_DBLCLKS;
        wndClass.lpfnWndProc = msgProc;
        wndClass.hInstance = hInstance;
        wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
        wndClass.lpszClassName = className;
        return wndClass;
    }

    static HWND GetCreatedWindow(HINSTANCE hInstance, LPWNDCLASSW wndClass, LPCWSTR windowTitle, UINT width, UINT height)
    {
        DebugPrintScope _(L"WindowScope::GetCreatedWindow");

        RECT rc;
        SetRect(&rc, 0, 0, width, height);
        AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

        return CreateWindowW(wndClass->lpszClassName, windowTitle, WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU, 100, 100, width, height, 0, (HMENU)nullptr, hInstance, 0);
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