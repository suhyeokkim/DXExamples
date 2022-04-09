#pragma once

#include <cstdio>

#include <windows.h>
#include <dxgidebug.h>
#include <d3d12sdklayers.h>

#include "symbols.h"

void CheckLeakForD3D12();

struct DECLSPEC_DLL DebugPrintScope
{
    const wchar_t* wcs;
    bool reportDx12Memory;

    DebugPrintScope(const wchar_t* wcs, bool reportDx12Memory = false);
    ~DebugPrintScope();
};