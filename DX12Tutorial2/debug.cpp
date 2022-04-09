#include "debug.h"


DebugPrintScope::DebugPrintScope(const wchar_t* wcs, bool reportDx12Memory) : wcs(wcs), reportDx12Memory(reportDx12Memory)
{
#if defined(_DEBUG)
    wprintf(L"[%s] start\n", wcs);
#endif
}

DebugPrintScope::~DebugPrintScope()
{
#if defined(_DEBUG)
    wprintf(L"[%s] end\n", wcs);

    if (reportDx12Memory) {
        CheckLeakForD3D12();
    }
#endif
}

#ifdef _DEBUG
void CheckLeakForD3D12()
{
    HMODULE dxgidebugdll = GetModuleHandleW(L"dxgidebug.dll");
    decltype(&DXGIGetDebugInterface) GetDebugInterface = reinterpret_cast<decltype(&DXGIGetDebugInterface)>(GetProcAddress(dxgidebugdll, "DXGIGetDebugInterface"));

    IDXGIDebug* debug;

    GetDebugInterface(IID_PPV_ARGS(&debug));

    OutputDebugStringW(L"▽▽▽▽▽▽▽▽▽▽ Direct3D Object ref count 메모리 누수 체크 ▽▽▽▽▽▽▽▽▽▽▽\r\n");
    debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_DETAIL);
    OutputDebugStringW(L"△△△△△△△△△△ 반환되지 않은 IUnknown 객체가 있을경우 위에 나타납니다. △△△△△△△△△△△△\r\n");

    debug->Release();
}
#endif
