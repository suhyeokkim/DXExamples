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
    DebugPrintScope _(L"GetDXWindowSetting");

    set->settings.windowName = L"DX12Tutorial";
    set->settings.windowWidth = 1024;
    set->settings.windowHeight = 768;
    set->settings.maxFrameRate = 144;
}

HRESULT EnableDebugLayer()
{
#if defined(_DEBUG)
    ID3D12Debug1* debugInterface;
    auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
    FAILED_RETURN(hr);

    // debugInterface->EnableDebugLayer();
    debugInterface->SetEnableGPUBasedValidation(true);
#endif

    return S_OK;
}

HRESULT DXEntryInit(WindowInstance* wnd, HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, uint32 maxFrameRate, bool debug)
{
    DebugPrintScope _(L"DXEntryInit");

    DXInstance* dx = &wnd->dx;
    *dx = { 0, };

    auto createFactoryFlags = (uint32)0;
    auto hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dx->dxgiFactory));
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create DXGIFactory..");

    dx->tearingSupport = CheckTearingSupport(dx->dxgiFactory);

    hr = D3D12CreateDevice(dx->dx12Device, D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(&dx->dx12Device));
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create D3D12Device..");

#if defined(_DEBUG)
    hr = dx->dx12Device->QueryInterface(IID_PPV_ARGS(&dx->infoQueue));
    if (SUCCEEDED(hr))
    {
        dx->infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
        dx->infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_ERROR, true);
        dx->infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY::D3D12_MESSAGE_SEVERITY_WARNING, true);

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

        hr = dx->infoQueue->PushStorageFilter(&infoFilter);
    }
#endif

    for (auto i = 0; i < g_CmdListCount; i++) {
        hr = CreateDXCommands(dx->dx12Device, dx->commands + i, dx->commandTypes[i]);
        FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create cmd list..");
    }

    auto backBufferCount = 3;
    hr = CreateSwapChain(dx->commands[0].queue, hWnd, width, height, backBufferCount, maxFrameRate, &dx->swapChain);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create swap chain..");

    auto rtv = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    auto rtvHeapPtr = dx->heaps + rtv;
    hr = CreateDescriptorHeap(dx->dx12Device, rtv, FRAME_COUNT, rtvHeapPtr);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create rtv descriptor heap..");

    hr = UpdateRenderTargetViews(dx->dx12Device, FRAME_COUNT, dx->swapChain, *rtvHeapPtr, dx->backBuffers);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create rtv descriptor heap..");

    dx->currentBackBufferIndex = 0;

    hr = EnableDebugLayer();
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to get debugInterface..");

    dx->vsync = true;

    return S_OK;
}

void DXEntryClean(WindowInstance* wnd)
{
    DXInstance* dx = &wnd->dx;

    DebugPrintScope _(L"DXEntryClean");

    for (auto i = 0; i < COMMANDS_COUNT; i++) {
        auto commandPtr = dx->commands + i;
        Flush(commandPtr->queue, commandPtr->fences[i], commandPtr->fenceValueSeq + i, commandPtr->fenceEvents[i]);

        DestroyDXCommands(commandPtr);
    }

    for (auto i = 0; i < D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES; i++) {
        SAFE_RELEASE(dx->heaps[i]);
    }

    SAFE_RELEASE(dx->infoQueue);
    SAFE_RELEASE(dx->swapChain);
    SAFE_RELEASE(dx->dxgiFactory);
    SAFE_RELEASE(dx->dx12Device);
}

HRESULT DXEntryResize(WindowInstance* wnd, uint32 width, uint32 height);

void DXEntryFrameUpdate(WindowInstance* wnd)
{
    // DebugPrintScope _(L"DXEntryFrameUpdate");

    if (g_ReserveWidth != 0 || g_ReserveHeight != 0) {
        DXEntryResize(wnd, g_ReserveWidth, g_ReserveHeight);
        g_ReserveWidth = 0; g_ReserveHeight = 0;
    }

    DXInstance* dx = &wnd->dx;

    auto commandIndex = 0;
    auto command = dx->commands[commandIndex];
    auto commandAllocator = command.allocators[0];
    auto commandList = command.commandList[0];

    auto backBuffer = dx->backBuffers[dx->currentBackBufferIndex];

    commandAllocator->Reset();
    commandList->Reset(commandAllocator, nullptr);

    auto rtvSize = dx->dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    {
        // 렌더타겟-버퍼 클리어
        D3D12_RESOURCE_BARRIER backBufferResBarrier = {};
        backBufferResBarrier.Transition.pResource = backBuffer;
        backBufferResBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        backBufferResBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        commandList->ResourceBarrier(1, &backBufferResBarrier);

        auto rtvHeaps = dx->heaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
        auto rtvHeapStart = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        D3D12_CPU_DESCRIPTOR_HANDLE rtv;
        auto rtvIndex = commandIndex;
        rtv.ptr = SIZE_T(rtvHeapStart.ptr + INT64(rtvSize) * INT64(rtvIndex));

        commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }

    // 여기서 해당 프레임 렌더링!?

    {
        // 스왑체인-버퍼 표시
        auto commandQueue = command.queue;

        D3D12_RESOURCE_BARRIER presentBarrier = {};
        presentBarrier.Transition.pResource = backBuffer;
        presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        presentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        commandList->ResourceBarrier(1, &presentBarrier);

        auto hr = commandList->Close();
        FAILED_RETURN_VOID(hr);

        ID3D12CommandList* const lists[] = { commandList };
        commandQueue->ExecuteCommandLists(_countof(lists), lists);

        auto fence = command.fences[0];
        auto backBufferValueSeqPtr = command.fenceValueSeq + 0;
        auto backBufferFenceValuePtr = dx->frameFenceValues + dx->currentBackBufferIndex;
        hr = Signal(commandQueue, fence, backBufferValueSeqPtr, backBufferFenceValuePtr);
        FAILED_RETURN_VOID(hr);

        uint32 vsync = dx->vsync ? 1 : 0;
        uint32 presentFlags = dx->tearingSupport && !vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;

        auto swapChain = dx->swapChain;
        hr = swapChain->Present(vsync, presentFlags);
        FAILED_RETURN_VOID(hr);

        dx->currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

        auto fenceEvent = command.fenceEvents[0];
        WaitForFenceValue(fence, *backBufferFenceValuePtr, fenceEvent);
    }
}

HRESULT DXEntryResize(WindowInstance* wnd, uint32 width, uint32 height)
{
    DebugPrintScope _(L"DXEntryResize");

    return S_OK;
}

HRESULT DXEntryReserveResize(uint32 width, uint32 height)
{
    DebugPrintScope _(L"DXEntryReserveResize");
    return S_OK;
}
