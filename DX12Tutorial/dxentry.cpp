#include <d3d12.h>
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXTex.h>

#include "defined_macro.h"
#include "dxentry.h"
#include "dx12lib.h"
#include "common.h"
#include "geometry.h"
#include "input.h"

#include <chrono>

void GetDXWindowSetting(OUT WindowInstance* set)
{
    set->settings.windowName = L"DX12Tutorial0";
    set->settings.windowWidth = 1024;
    set->settings.windowHeight = 768;
    set->settings.maxFrameRate = 144;
}

HRESULT DXEntryInit(DXInstance* inst, HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, uint32 maxFrameRate, bool debug)
{
    memset(inst, 0, sizeof(inst));
    auto hr = (HRESULT)0;

    auto createFactoryFlags = (uint32)0;
    hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&inst->dxgiFactory));
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create DXGIFactory..");

    inst->tearingSupport = CheckTearingSupport(inst->dxgiFactory);

    hr = D3D12CreateDevice(inst->dx12Device, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&inst->dx12Device));
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create D3D12Device..");

#if defined(_DEBUG)
    hr = inst->dx12Device->QueryInterface(IID_PPV_ARGS(&inst->infoQueue));
    if (SUCCEEDED(hr))
    {
        inst->infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        inst->infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_ERROR, true);
        inst->infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_WARNING, true);

        // Suppress whole categories of messages
        //D3D12_MESSAGE_CATEGORY Categories[] = {};

        // Suppress messages based on their severity level
        D3D12_MESSAGE_SEVERITY severities[] =
        {
            D3D12_MESSAGE_SEVERITY_INFO
        };

        // Suppress individual messages by their ID
        D3D12_MESSAGE_ID denyIds[] = {
            D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
            D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
            D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
        };

        D3D12_INFO_QUEUE_FILTER infoFilter = {};
        //NewFilter.DenyList.NumCategories = _countof(Categories);
        //NewFilter.DenyList.pCategoryList = Categories;
        infoFilter.DenyList.NumSeverities = _countof(severities);
        infoFilter.DenyList.pSeverityList = severities;
        infoFilter.DenyList.NumIDs = _countof(denyIds);
        infoFilter.DenyList.pIDList = denyIds;

        hr = inst->infoQueue->PushStorageFilter(&infoFilter);
    }
#endif

    for (auto i = 0; i < g_CmdListCount; i++) {
        hr = CreateDXCommands(inst->dx12Device, inst->commands + i, inst->commandTypes[i]);
        FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create cmd list..");
    }

    auto backBufferCount = 3;
    hr = CreateSwapChain(inst->commands[0].queue, hWnd, width, height, backBufferCount, maxFrameRate, &inst->swapChain);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create swap chain..");

    auto rtv = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    auto rtvHeapPtr = inst->heaps + rtv;
    hr = CreateDescriptorHeap(inst->dx12Device, rtv, FRAME_COUNT, rtvHeapPtr);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create rtv descriptor heap..");

    hr = UpdateRenderTargetViews(inst->dx12Device, FRAME_COUNT, inst->swapChain, *rtvHeapPtr, inst->backBuffers);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create rtv descriptor heap..");

    return S_OK;
}

void DXEntryClean(DXInstance* dx)
{
    for (auto i = 0; i < COMMANDS_COUNT; i++) {
        DestroyDXCommands(dx->commands + i);
    }

    for (auto i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        SAFE_RELEASE(dx->heaps[i]);
    }

    SAFE_RELEASE(dx->infoQueue);
    SAFE_RELEASE(dx->swapChain);
    SAFE_RELEASE(dx->dxgiFactory);
    SAFE_RELEASE(dx->dx12Device);
}

void DXEntryFrameUpdate(DXInstance* dx)
{

}

HRESULT DXEntryResize(uint32 width, uint32 height)
{
    return S_OK;
}
