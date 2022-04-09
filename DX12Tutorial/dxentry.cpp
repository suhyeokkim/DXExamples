#include <d3d12.h>
#include <d3dx12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include <DirectXTex.h>
#include <pix.h>

#include "defined_macro.h"
#include "dxentry.h"
#include "dx12lib.h"
#include "common.h"
#include "geometry.h"
#include "input.h"
#include "debug.h"
#include "root.h"

#include <chrono>

void UpdateWindowIfDiff(Root* root)
{
    auto wndctx = &root->wndctx;

    if (wndctx->reserveWidth != 0 || wndctx->reserveHeight != 0) {
        DXEntryResize(root, wndctx->reserveWidth, wndctx->reserveHeight);
        wndctx->reserveWidth = 0; wndctx->reserveHeight = 0;
    }

    if (wndctx->fullScreen != root->settings.fullScreen) {
        DXEntryFullScreen(root, wndctx->fullScreen);
    }
}

void GetDXWindowSetting(WindowSetting* set, WindowContext* ctx)
{
    DebugPrintScope _(L"GetDXWindowSetting");

    set->windowName = L"DX12Tutorial";
    set->windowWidth = 1024;
    set->windowHeight = 768;
    set->maxFrameRate = 144;
    set->fullScreen = false;
    set->windowStyle = WS_OVERLAPPEDWINDOW;

    ctx->reserveWidth = 0;
    ctx->reserveHeight = 0;
    ctx->fullScreen = false;
}

HRESULT EnableDebugLayer()
{
#if defined(_DEBUG)
    ID3D12Debug1* debugInterface;
    auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
    FAILED_RETURN(hr);

    // debugInterface->EnableDebugLayer();
    // debugInterface->SetEnableGPUBasedValidation(true);
#endif

    return S_OK;
}

HRESULT DXEntryInit(Root* root, HINSTANCE hInstance, HWND hWnd, UINT width, UINT height, uint32 maxFrameRate, bool debug)
{
    DebugPrintScope _(L"DXEntryInit");

    WindowInstance* wnd = &root->wnd;
    DXInstance* dx = &root->dx;
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

    for (auto i = 0; i < CMDLIST_COUNT; i++) {
        hr = CreateDXCommands(dx->dx12Device, dx->commands + i, dx->commandTypes[i]);
        FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to create cmd list..");
    }

    hr = CreateSwapChain(dx->commands[0].queue, hWnd, width, height, FRAME_COUNT, maxFrameRate, &dx->swapChain);
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

void DXEntryClean(Root* root)
{
    DXInstance* dx = &root->dx;

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


HRESULT DXEntryFrameUpdate(Root* root)
{
    // DebugPrintScope _(L"DXEntryFrameUpdate");

    UpdateWindowIfDiff(root);

    DXInstance* dx = &root->dx;

    auto commandIndex = 0;
    auto command = dx->commands[commandIndex];
    auto commandAllocator = command.allocators[0];
    auto commandList = command.commandList[0];

    auto backBuffer = dx->backBuffers[dx->currentBackBufferIndex];

    auto hr = commandAllocator->Reset();
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to reset allocator..");

    hr = commandList->Reset(commandAllocator, nullptr);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to reset commandList..");

    auto rtvSize = dx->dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    {
        // 렌더타겟-버퍼 클리어
        D3D12_RESOURCE_BARRIER backBufferResBarrier = {};
        backBufferResBarrier.Transition.pResource = backBuffer;
        backBufferResBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        backBufferResBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

        commandList->ResourceBarrier(1, &backBufferResBarrier);

        auto rtvHeaps = dx->heaps[D3D12_DESCRIPTOR_HEAP_TYPE_RTV];
        auto rtvHeapStart = rtvHeaps->GetCPUDescriptorHandleForHeapStart();
        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        D3D12_CPU_DESCRIPTOR_HANDLE rtv;
        auto rtvIndex = dx->currentBackBufferIndex;
        rtv.ptr = SIZE_T(rtvHeapStart.ptr + INT64(rtvSize) * INT64(rtvIndex));

        commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
    }

    // 여기서 해당 프레임 렌더링!?

    {
        // 스왑체인-버퍼 표시
        auto commandQueue = command.queue;

        D3D12_RESOURCE_BARRIER presentBarrier = {};
        presentBarrier.Transition.pResource = backBuffer;
        presentBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        presentBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

        commandList->ResourceBarrier(1, &presentBarrier);

        hr = commandList->Close();
        FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to close cmdlist..");

        ID3D12CommandList* const lists[] = { commandList };
        commandQueue->ExecuteCommandLists(_countof(lists), lists);

        auto fence = command.fences[0];
        auto backBufferValueSeqPtr = command.fenceValueSeq + 0;
        auto backBufferFenceValuePtr = dx->frameFenceValues + dx->currentBackBufferIndex;
        hr = Signal(commandQueue, fence, backBufferValueSeqPtr, backBufferFenceValuePtr);
        FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to signal..");

        uint32 vsync = dx->vsync ? 1 : 0;
        uint32 presentFlags = dx->tearingSupport && !vsync ? DXGI_PRESENT_ALLOW_TEARING : 0;

        auto swapChain = dx->swapChain;
        hr = swapChain->Present(vsync, presentFlags);
        FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to present..");

        dx->currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

        auto fenceEvent = command.fenceEvents[0];
        hr = WaitForFenceValue(fence, *backBufferFenceValuePtr, fenceEvent);
        FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to wait fence..");
    }

    return S_OK;
}

HRESULT DXEntryResize(Root* root, uint32 width, uint32 height)
{
    DebugPrintScope _(L"DXEntryResize");

    root->settings.windowWidth = width;
    root->settings.windowHeight = height;

    DXInstance* dx = &root->dx;
    DXCommands* command = dx->commands;
    auto queue = command->queue;
    auto fence = command->fences[0];
    auto frameValueSeqPtr = command->fenceValueSeq + 0;
    auto backBufferFenceValuePtr = dx->frameFenceValues + dx->currentBackBufferIndex;

    auto hr = Flush(queue, fence, frameValueSeqPtr, backBufferFenceValuePtr);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to flush for resize..");

    for (auto i = 0; i < FRAME_COUNT; i++) {
        auto backBuffer = dx->backBuffers[i];
        backBuffer->Release();
        dx->backBuffers[i] = nullptr;
        
        dx->frameFenceValues[i] = dx->frameFenceValues[dx->currentBackBufferIndex];
    }

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

    auto swapChain = dx->swapChain;
    hr = swapChain->GetDesc(&swapChainDesc);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to get swapchain desc..");
    hr = swapChain->ResizeBuffers(
        FRAME_COUNT, width, height, 
        swapChainDesc.BufferDesc.Format, swapChainDesc.Flags
    );
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to resize buffers..");
    
    dx->currentBackBufferIndex = swapChain->GetCurrentBackBufferIndex();

    auto rtv = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    hr = UpdateRenderTargetViews(dx->dx12Device, FRAME_COUNT, swapChain, dx->heaps[rtv], dx->backBuffers);
    FAILED_ERROR_MESSAGE_RETURN(hr, L"fail to update rtv..");

    return S_OK;
}

HRESULT DXEntryFullScreen(Root* root, bool fullScreen)
{
    root->settings.fullScreen = fullScreen;
    auto wnd = &root->wnd;

    if (fullScreen) {
        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::GetWindowRect(wnd->hWnd, &wnd->rect), L"failed to GetWindowRect..", E_FAIL
        );

        auto fullScreenStyle = 
            WS_OVERLAPPEDWINDOW &
            ~(WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::SetWindowLongW(wnd->hWnd, GWL_STYLE, fullScreenStyle), 
            L"failed to SetWindowLongW..", E_FAIL
        );

        auto hMonitor = ::MonitorFromWindow(wnd->hWnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFOEX monitorInfo = {};
        monitorInfo.cbSize = sizeof(MONITORINFOEX);
        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::GetMonitorInfoW(hMonitor, &monitorInfo), L"failed to GetMonitorInfoW..", E_FAIL
        );

        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::SetWindowPos(wnd->hWnd, HWND_TOPMOST,
                monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.top,
                monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left,
                monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE
            ), L"failed to SetWindowPos..", E_FAIL
        );

        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::ShowWindow(wnd->hWnd, SW_SHOW), L"failed to ShowWindow..", E_FAIL
        );
    } else {
        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::SetWindowLongW(wnd->hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW),
            L"failed to SetWindowLongW..", E_FAIL
        );

        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::SetWindowPos(wnd->hWnd, HWND_TOPMOST,
                wnd->rect.left,
                wnd->rect.top,
                wnd->rect.right - wnd->rect.left,
                wnd->rect.bottom - wnd->rect.top,
                SWP_FRAMECHANGED | SWP_NOACTIVATE
            ), L"failed to SetWindowPos..", E_FAIL
        );
        FALSE_ERROR_MESSAGE_RETURN_CODE(
            ::ShowWindow(wnd->hWnd, SW_SHOW), L"failed to ShowWindow..", E_FAIL
        );
    }

    return S_OK;
}

HRESULT DXEntryReserveResize(Root* root,uint32 width, uint32 height)
{
    DebugPrintScope _(L"DXEntryReserveResize");

    auto& wndctx = root->wndctx;
    wndctx.reserveWidth = width;
    wndctx.reserveHeight = height;
    return S_OK;
}

HRESULT DXEntryToggleFullscreen(Root* root)
{
    auto& wndctx = root->wndctx;
    wndctx.fullScreen = !wndctx.fullScreen;
    return S_OK;
}