#pragma once

#include "root.h"
#include "winmain.h"

void GetDXWindowSetting(WindowSetting* set, WindowContext* ctx);
HRESULT DXEntryInit(Root* root, HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, uint32 maxFrameRate, bool debug);
void DXEntryClean(Root* root);
HRESULT DXEntryFrameUpdate(Root* root);
HRESULT DXEntryReserveResize(Root* root, uint32 width, uint32 height);
HRESULT DXEntryToggleFullscreen(Root* root);
HRESULT DXEntryResize(Root* root, uint32 width, uint32 height);
HRESULT DXEntryFullScreen(Root* root, bool fullScreen);