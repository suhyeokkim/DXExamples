#include "debug.h"

DebugPrintScope::DebugPrintScope(const wchar_t* wcs) : wcs(wcs)
{
#if defined(_DEBUG)
    wprintf(L"[%s] ok\n", wcs);
#endif
}

DebugPrintScope::~DebugPrintScope()
{
#if defined(_DEBUG)
    wprintf(L"[%s] off\n", wcs);
#endif
}
