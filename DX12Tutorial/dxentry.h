#pragma once

#include "winmain.h"
#include "container.h"

void GetDXWindowSetting(OUT WindowInstance* set);
HRESULT DXEntryInit(DXInstance* dxInstance, HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, uint32 maxFrameRate, bool debug);
void DXEntryClean(DXInstance* dx);
void DXEntryFrameUpdate(DXInstance* dx);
HRESULT DXEntryResize(UINT width, UINT height);
