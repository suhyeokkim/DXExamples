#pragma once

#include "defined_type.h"

#define NOMINMAX
#include <d3d12.h>

constexpr int CMDLIST_COUNT = 3;

struct DXCommands
{
    ID3D12CommandQueue* queue;
    ID3D12CommandAllocator* allocators[CMDLIST_COUNT];
    ID3D12GraphicsCommandList* commandList[CMDLIST_COUNT];
    ID3D12Fence* fences[CMDLIST_COUNT];
    HANDLE fenceEvents[CMDLIST_COUNT];
    uint64 fenceValueSeq[CMDLIST_COUNT];
};

HRESULT CreateDXCommands(ID3D12Device2* device, DXCommands* commands, D3D12_COMMAND_LIST_TYPE type);
HRESULT DestroyDXCommands(DXCommands* commands);