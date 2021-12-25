#pragma once

#include "defined_type.h"
#include "dx12cmd.h"

#define NOMINMAX
#include <d3d12.h>
#include <dxgi1_6.h>

#include <chrono>

#define FRAME_COUNT 3
#define COMMANDS_COUNT 3

struct DXCommands;
struct DXInstance
{
    ID3D12Device2* dx12Device;
    ID3D12InfoQueue* infoQueue;

    ID3D12DescriptorHeap* heaps[D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES];

    bool tearingSupport;
    IDXGIFactory4* dxgiFactory;
    IDXGISwapChain4* swapChain;
    ID3D12Resource* backBuffers[FRAME_COUNT];

    D3D12_COMMAND_LIST_TYPE commandTypes[COMMANDS_COUNT] = { 
        D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT,
        D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COMPUTE,
        D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_COPY
    };
    DXCommands commands[COMMANDS_COUNT];
};

bool CheckTearingSupport(IDXGIFactory4* factory);
HRESULT CreateSwapChain(ID3D12CommandQueue* cmdQueue, HWND hWnd, uint32 width, uint32 height, uint32 bufferCount, uint32 maxFrameRate, IDXGISwapChain4** outSwapChain);
HRESULT CreateDescriptorHeap(ID3D12Device* dx12Device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32 numDescriptors, ID3D12DescriptorHeap** outHeap);
HRESULT UpdateRenderTargetViews(ID3D12Device2* dx12Device, int32 backBufferCount, IDXGISwapChain4* outSwapChain, ID3D12DescriptorHeap* heap, ID3D12Resource** outBackBuffer);

HRESULT CreateCommandQueue(IN ID3D12Device2* device, IN D3D12_COMMAND_LIST_TYPE type, OUT ID3D12CommandQueue** outCommandQueue);
HRESULT CreateCommandAllocator(ID3D12Device2* dx12Device, D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator** outAllocator);
HRESULT CreateCommandList(ID3D12Device2* dx12Device, ID3D12CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type, ID3D12GraphicsCommandList** outCmdList);
HRESULT CreateFense(ID3D12Device2* dx12Device, ID3D12Fence** outFence);
HRESULT CreateEventHandle();
HRESULT DestroyEventHandle(HANDLE event);
HRESULT Signal(ID3D12CommandQueue* cmdQueue, ID3D12Fence* outFence, uint64* originFenceValue, uint64* outFenceValue);
HRESULT WaitForFenceValue(ID3D12Fence* outFence, uint64 waitFenceValue, HANDLE fenceEvent, std::chrono::milliseconds duration);
HRESULT WaitForFenceValue(ID3D12Fence* outFence, uint64 waitFenceValue, HANDLE fenceEvent);
HRESULT Flush(ID3D12CommandQueue* cmdQueue, ID3D12Fence* outFence, uint64* fenceValue, HANDLE fenceEvent);
