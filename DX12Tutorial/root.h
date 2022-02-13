#pragma once

#include "dx12lib.h"
#include "winmain.h"

struct Root
{
    WindowSetting settings;
    WindowInstance wnd;
    DXInstance dx;
    WindowContext wndctx;
};
