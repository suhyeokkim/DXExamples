#pragma once

#include "dx12lib.h"

struct WindowSetting
{
    const WCHAR* windowName;
    UINT windowWidth;
    UINT windowHeight;
    UINT maxFrameRate;
};

struct DXInstance;
struct WindowInstance
{
    WNDCLASSW wndClass;
    HWND hWnd;
    WindowSetting settings;
    DXInstance dx;
};