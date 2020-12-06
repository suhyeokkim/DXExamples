#include <windows.h>

#pragma once

HRESULT DXEntryInit(HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, UINT maxFrameRate, bool debug);
void DXEntryClean();
void DXEntryFrameUpdate();
