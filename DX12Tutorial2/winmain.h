#pragma once

#include "dx12lib.h"

struct WindowSetting
{
    const WCHAR* windowName;
    UINT windowWidth;
    UINT windowHeight;
    UINT maxFrameRate;
    bool fullScreen;
    DWORD windowStyle;
};

struct DXInstance;
struct WindowInstance
{
    WNDCLASSW wndClass;
    HWND hWnd;
    RECT rect;
    WindowSetting settings;
    DXInstance dx;
};