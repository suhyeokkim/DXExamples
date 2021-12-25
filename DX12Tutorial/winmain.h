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
    WindowSetting settings;
    DXInstance dx;
};