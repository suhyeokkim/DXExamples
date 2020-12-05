#include <windows.h>
#include <iostream>
#include <fcntl.h> 
#include <cstdio>

#include "defined.h"
#include "dxentry.h"

using namespace std;

void RedirectIOToConsole(int maxLineOfConsole = 500)
{
	AllocConsole();

	FILE* stream;
	freopen_s(&stream, "CONIN$", "r", stdin);
	freopen_s(&stream, "CONOUT$", "w", stdout);
	freopen_s(&stream, "CONOUT$", "w", stderr);
}

LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
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

inline WNDCLASSW GetWindowClass(HINSTANCE hInstance, WNDPROC msgProc, LPCWSTR className)
{
	WNDCLASSW wndClass;
	memset(&wndClass, 0, sizeof(WNDCLASS));
	wndClass.style = CS_DBLCLKS;
	wndClass.lpfnWndProc = msgProc;
	wndClass.hInstance = hInstance;
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszClassName = className;
	return wndClass;
}

inline HWND GetCreatedWindow(HINSTANCE hInstance, LPWNDCLASSW wndClass, LPCWSTR windowTitle, UINT width, UINT height)
{
	RECT rc;
	SetRect(&rc, 0, 0, width, height);
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	return CreateWindowW(wndClass->lpszClassName, windowTitle, WS_POPUP | WS_BORDER | WS_CAPTION | WS_SYSMENU, 100, 100, width, height, 0, (HMENU)nullptr, hInstance, 0);
}

inline void WindowLoop(HWND hWnd)
{
	MSG msg;
	while (true)
	{
		if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				break; 
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			UpdateWindow(hWnd);
		}
		else
		{
			DXEntryFrameUpdate();
		}
	}
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	RedirectIOToConsole();

	// Register the windows class
	WNDCLASSW wndClass = GetWindowClass(hInstance, MsgProc, L"Direct3DWindowClass");

	FALSE_MESSAGE_RETURN(RegisterClassW(&wndClass), L"fail to register window class..");

	UINT nDefaultWidth = 1024, nDefaultHeight = 768, maxFrameRate = 144;

	WCHAR strWindowTitle[] = L"Test";
	HWND hWnd = GetCreatedWindow(hInstance, &wndClass, strWindowTitle, nDefaultWidth, nDefaultHeight);

	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
	
	FAILED_MESSAGE_RETURN(
		DXEntryInit(hInstance, hWnd, nDefaultWidth, nDefaultHeight, maxFrameRate, true),
		L"Fail to initialize."
	);

	WindowLoop(hWnd);

	DXEntryClean();
	FreeConsole();

	return 0;
}