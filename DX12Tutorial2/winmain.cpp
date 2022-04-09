#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <winuser.h>
#include <iostream>
#include <fcntl.h> 
#include <cstdio>

#include "winmain.h"
#include "dxentry.h"
#include "defined_macro.h"
#include "root.h"
#include "debug.h"

#include <filesystem>
#include <shlobj.h>

// https://devblogs.microsoft.com/pix/taking-a-capture/
static std::wstring GetLatestWinPixGpuCapturerPath()
{
    LPWSTR programFilesPath = nullptr;
    SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

    std::filesystem::path pixInstallationPath = programFilesPath;
    pixInstallationPath /= "Microsoft PIX";

    std::wstring newestVersionFound;

    for (auto const& directory_entry : std::filesystem::directory_iterator(pixInstallationPath))
    {
        if (directory_entry.is_directory())
        {
            if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
            {
                newestVersionFound = directory_entry.path().filename().c_str();
            }
        }
    }

    if (newestVersionFound.empty())
    {
        // TODO: Error, no PIX installation found
    }

    return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
}


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
    Root* root;

    HRESULT Init(HINSTANCE hInstance, Root* root)
    {
        DebugPrintScope _(L"WindowScope::Init");
        WindowInstance& wndInst = root->wnd;
        WindowSetting& wndSet = root->settings;
        WindowContext& wndCtx = root->wndctx;

        // Windows 10 Creators update adds Per Monitor V2 DPI awareness context.
        // Using this awareness context allows the client area of the window 
        // to achieve 100% scaling while still allowing non-client window content to 
        // be rendered in a DPI sensitive fashion.
        SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

        // Register the windows class
        wndInst.wndClass = GetWindowClass(hInstance, MsgProc, L"Direct3DWindowClass");
        FALSE_ERROR_MESSAGE_RETURN_CODE(RegisterClassW(&wndInst.wndClass), L"fail to register window class..", E_FAIL);

        GetDXWindowSetting(&wndSet, &wndCtx);
        wndInst.hWnd = GetCreatedWindow(hInstance, &wndInst.wndClass, wndSet.windowName, wndSet.windowStyle, wndSet.windowWidth, wndSet.windowHeight);

        SetWindowLongPtr(wndInst.hWnd, GWLP_USERDATA, (LONG_PTR)root);

        ShowWindow(wndInst.hWnd, SW_SHOW);
        UpdateWindow(wndInst.hWnd);

        FAILED_ERROR_MESSAGE_RETURN(
            DXEntryInit(root, hInstance, wndInst.hWnd, wndSet.windowWidth, wndSet.windowHeight, wndSet.maxFrameRate, true),
            L"fail to initialize."
        );

        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::GetWindowRect(wndInst.hWnd, &wndInst.rect), L"failed to GetWindowRect..", E_FAIL
        );


        this->root = root;

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
                DXEntryFrameUpdate(root);
            }
        }
    }

    ~WindowScope()
    {
        DebugPrintScope _(L"~WindowScope");

        DXEntryClean(root);
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

        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
            switch (wParam)
            {
            case VK_F11:
            {
                auto rootPtr = (Root*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
                DXEntryToggleFullscreen(rootPtr);
            }
            break;
            default:
                return DefWindowProc(hWnd, uMsg, wParam, lParam);
            }

        case WM_SIZE:
        {
            RECT clientRect = {};
            ::GetClientRect(hWnd, &clientRect);

            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;

            auto rootPtr = (Root*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            DXEntryReserveResize(rootPtr, width, height);
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

int main(int argc, char** argv)
{
    RedirectConsoleScope _;

    // Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
    // This may happen if the application is launched through the PIX UI. 
    if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
    {
        LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
    }

    {
        Root root;
        memset(&root, 0, sizeof(root));
        WindowScope wndScope;
        auto processInatance = GetModuleHandle(NULL);

        if (FAILED(wndScope.Init(processInatance, &root)))
        {
            goto END;
        }

        wndScope.Loop();
    }

END:

#if defined(_DEBUG)
    puts("[DEBUG] press any key to end program..");
    auto var = getchar();
#endif

    return 0;
}