#pragma once

#include <windows.h>
#include "defined_type.h"

struct WindowSetting
{
    const WCHAR* windowName;
    UINT windowWidth;
    UINT windowHeight;
    UINT maxFrameRate;
    bool fullScreen;
    DWORD windowStyle;
};

struct WindowInstance
{
    WNDCLASSW wndClass;
    HWND hWnd;
    RECT rect;
};

struct WindowContext
{
    uint32 reserveWidth;
    uint32 reserveHeight;
    bool fullScreen;
};