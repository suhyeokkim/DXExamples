#pragma once

#include "winmain.h"
#include "container.h"

void GetDXWindowSetting(OUT WindowSetting* set);
HRESULT DXEntryInit(WindowInstance* wnd, HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, uint32 maxFrameRate, bool debug);
void DXEntryClean(WindowInstance* wnd);
void DXEntryFrameUpdate(WindowInstance* wnd);
HRESULT DXEntryReserveResize(UINT width, UINT height);
