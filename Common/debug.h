#pragma once

#include <cstdio>
#include "symbols.h"

struct DECLSPEC_DLL DebugPrintScope
{
    const wchar_t* wcs;

    DebugPrintScope(const wchar_t* wcs);
    ~DebugPrintScope();
};