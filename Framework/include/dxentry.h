#pragma once

#include "winmain.h"

void GetDXWindowSetting(OUT WindowSetting* set);
HRESULT DXEntryInit(HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, UINT maxFrameRate, bool debug);
void DXEntryClean();
void DXEntryFrameUpdate();
HRESULT DXEntryResize(UINT width, UINT height);
