#pragma once

#include "res.h"
#include "dx12lib.h"
#include "winmain.h"
#include "context.h"

struct Root
{
    WindowSetting settings;
    Resource res;
    WindowInstance wnd;
    DXInstance dx;
    WindowContext wndctx;
};
