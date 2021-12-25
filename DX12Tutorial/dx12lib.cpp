#include "dx12lib.h"

#include "defined_macro.h"

#include <chrono>

bool CheckTearingSupport(IDXGIFactory4* factory)
{
    IDXGIFactory7* dxgiFactory7;
    if (SUCCEEDED(factory->QueryInterface(IID_PPV_ARGS(&dxgiFactory7))))
    {
        BOOL check = FALSE;
        if (FAILED(dxgiFactory7->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING, &check, sizeof(check))))
        {
            check = FALSE;
        }

        return check == TRUE;
    }
    else
    {
        return false;
    }
}

HRESULT CreateSwapChain(ID3D12CommandQueue* cmdQueue, HWND hWnd, uint32 width, uint32 height, uint32 bufferCount, uint32 maxFrameRate, IDXGISwapChain4** outSwapChain)
{
    HRESULT hr;
    IDXGISwapChain4* dxgiSwapChain4 = nullptr;
    IDXGIFactory4* dxgiFactory4 = nullptr;
    uint32 createFactoryFlags = 0;
#if defined (_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    hr = CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4));
    if (FAILED(hr))
        return hr;

    hr = dxgiFactory4->MakeWindowAssociation(hWnd, DXGI_MWA_NO_ALT_ENTER);
    if (FAILED(hr))
        return hr;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = bufferCount;
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    swapChainDesc.Flags = CheckTearingSupport(dxgiFactory4) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;

    IDXGISwapChain1* swapChain1 = nullptr;
    hr = dxgiFactory4->CreateSwapChainForHwnd(cmdQueue, hWnd, &swapChainDesc, nullptr, nullptr, &swapChain1);
    if (FAILED(hr))
        return hr;

    IDXGISwapChain4* swapChain4 = nullptr;
    hr = swapChain1->QueryInterface(IID_PPV_ARGS(&swapChain4));
    if (FAILED(hr))
        return hr;

    *outSwapChain = swapChain4;
    return S_OK;
}

HRESULT CreateDescriptorHeap(ID3D12Device* dx12Device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors, ID3D12DescriptorHeap** outHeap)
{
    HRESULT hr;
    D3D12_DESCRIPTOR_HEAP_DESC desc;
    desc.NumDescriptors = numDescriptors;
    desc.Type = type;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    desc.NodeMask = 0;

    hr = dx12Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(outHeap));
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT UpdateRenderTargetViews(ID3D12Device2* dx12Device, int32 backBufferCount, IDXGISwapChain4* outSwapChain, ID3D12DescriptorHeap* heap, ID3D12Resource** outBackBuffer)
{
    HRESULT hr;
    auto rtvDescSize = dx12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = heap->GetCPUDescriptorHandleForHeapStart();
    for (auto i = 0; i < backBufferCount; i++)
    {
        ID3D12Resource* backBuffer = nullptr;
        hr = outSwapChain->GetBuffer(i, IID_PPV_ARGS(&backBuffer));
        if (FAILED(hr)) return hr;

        dx12Device->CreateRenderTargetView(backBuffer, nullptr, rtvHandle);

        outBackBuffer[i] = backBuffer;

        rtvHandle.ptr = SIZE_T(INT64(rtvHandle.ptr) + INT64(rtvDescSize));
    }

    return S_OK;
}


/// <summary>
/// 커맨드 큐, 타임아웃은 제거, NORMAL 기본, GPU 하나 기준
/// </summary>
HRESULT CreateCommandQueue(IN ID3D12Device2* device, IN D3D12_COMMAND_LIST_TYPE type, OUT ID3D12CommandQueue** outCommandQueue)
{
    D3D12_COMMAND_QUEUE_DESC desc = {};
    desc.Type = type;
    desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY::D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
    desc.Flags = D3D12_COMMAND_QUEUE_FLAGS::D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT;
    desc.NodeMask = 0;

    return device->CreateCommandQueue(&desc, IID_PPV_ARGS(outCommandQueue));
}

/// <summary>
/// 커맨드 알로케이터에서 Reset 을 사용해 재사용 하기 위해선
/// GPU 내에서 전부 사용이 되어있어야함. 그래서 fense 사용 필요.
/// commandlist -> 1:1
/// D3D12_COMMAND_LIST_TYPE_BUNDLE : inherit gpu state (except PSO, topology)
/// </summary>
HRESULT CreateCommandAllocator(ID3D12Device2* dx12Device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator** outAllocator)
{
    return dx12Device->CreateCommandAllocator(type, IID_PPV_ARGS(outAllocator));
}

/// <summary>
/// 커맨드 리스트 만들기, close 를 해줘야 나중에 reset 으로 시작 가능
/// </summary>
HRESULT CreateCommandList(ID3D12Device2* dx12Device, ID3D12CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** outCmdList)
{
    auto hr = dx12Device->CreateCommandList(0, type, commandAllocator, nullptr, IID_PPV_ARGS(outCmdList));
    FAILED_RETURN(hr);

    return (*outCmdList)->Close();
}

/// <summary>
/// 펜스 만들기
/// proc: Signal                     / Wait
/// cpu : ID3D12Fence::Signal        / ID3D12Fense::SetEventOnCompletion + 
///                                  / WaitForSingleObject
/// gpu : ID3D12CommandQueue::Signal / ID3D12CommandQueue::Wait
/// ID3D12Fence::SetEventOnCompletion
/// </summary>
HRESULT CreateFense(ID3D12Device2* dx12Device, ID3D12Fence** outFence)
{
    return dx12Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(outFence));
}

/// <summary>
/// 이벤트 핸들 만들어잉
/// </summary>
HRESULT CreateEventHandle()
{
    HANDLE fenceEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);

    if (!fenceEvent) {
        return S_FALSE;
    }

    return S_OK;
}

/// <summary>
/// 이벤트 핸들 제거해잉
/// </summary>
HRESULT DestroyEventHandle(HANDLE event)
{
    if (::CloseHandle(event)) {
        return S_OK;
    }
    else {
        return E_FAIL;
    }
}

/// <summary>
/// GPU 시그날
/// </summary>
HRESULT Signal(ID3D12CommandQueue* cmdQueue, ID3D12Fence* outFence, uint64* originFenceValue, uint64* outFenceValue)
{
    *outFenceValue = ++(*originFenceValue);
    return cmdQueue->Signal(outFence, *outFenceValue);
}

/// <summary>
/// GPU Fence Value Query
/// </summary>
HRESULT WaitForFenceValue(ID3D12Fence* outFence, uint64 waitFenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration)
{
    if (outFence->GetCompletedValue() < waitFenceValue) {
        auto hr = outFence->SetEventOnCompletion(waitFenceValue, fenceEvent);
        FAILED_RETURN(hr);

        // conditional_variable, ctxt_switch cost
        hr = ::WaitForSingleObject(fenceEvent, scast<DWORD>(duration.count()));

        switch (hr)
        {
        case WAIT_OBJECT_0:
            break;
        case WAIT_TIMEOUT:
            break;
        case WAIT_FAILED:
            return WAIT_FAILED;
        case WAIT_ABANDONED:
            assert(false || "WAIT_ABANDONED returned");
        }
    }

    return S_OK;
}

HRESULT WaitForFenceValue(ID3D12Fence* outFence, uint64 waitFenceValue, HANDLE fenceEvent)
{
    auto duration = std::chrono::milliseconds::max();
    return WaitForFenceValue(outFence, waitFenceValue, fenceEvent, duration);
}

/// <summary>
/// GPU 에 커맨드가 끝날 때까지 대기
/// </summary>
HRESULT Flush(ID3D12CommandQueue* cmdQueue, ID3D12Fence* outFence, uint64* fenceValue, HANDLE fenceEvent)
{
    uint64 fenceValueForSignal = 0;

    auto hr = Signal(cmdQueue, outFence, fenceValue, &fenceValueForSignal);
    FAILED_RETURN(hr);

    return WaitForFenceValue(outFence, fenceValueForSignal, fenceEvent);
}