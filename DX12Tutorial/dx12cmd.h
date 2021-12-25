#pragma once

#include "defined_type.h"

#define NOMINMAX
#include <d3d12.h>

const int g_CmdListCount = 3;

struct DXCommands
{
    ID3D12CommandQueue* queue;
    ID3D12CommandAllocator* allocator[g_CmdListCount];
    ID3D12GraphicsCommandList* commandList[g_CmdListCount];
    ID3D12Fence* fences[g_CmdListCount];
    HANDLE* fenceEvents[g_CmdListCount];
};

HRESULT CreateDXCommands(ID3D12Device2* device, DXCommands* commands, D3D12_COMMAND_LIST_TYPE type);
HRESULT DestroyDXCommands(DXCommands* commands);