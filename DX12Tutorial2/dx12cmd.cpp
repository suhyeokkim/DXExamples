#include "dx12cmd.h"
#include "dx12lib.h"
#include "defined_macro.h"

#include <chrono>

/// <summary>
/// 커맨드 큐 하나에서 사용할 것
/// </summary>
HRESULT CreateDXCommands(ID3D12Device2* device, DXCommands* commands, D3D12_COMMAND_LIST_TYPE type)
{
    memset(commands, 0, sizeof(DXCommands));

    auto hr = (HRESULT)0;
    hr = CreateCommandQueue(device, type, &commands->queue);
    FAILED_ERROR_MESSAGE_GOTO(hr, L"fail to create qeueu in DXCommands..", CREATEDXCOMMANDS_CLEAR);

    for (auto i = 0; i < CMDLIST_COUNT; i++) {
        hr = CreateCommandAllocator(device, type, commands->allocators + i);
        FAILED_ERROR_MESSAGE_GOTO(hr, L"fail to create dx12allocator in DXCommands..", CREATEDXCOMMANDS_CLEAR);

        hr = CreateCommandList(device, commands->allocators[i], type, commands->commandList + i);
        FAILED_ERROR_MESSAGE_GOTO(hr, L"fail to create dx12commandlist in DXCommands..", CREATEDXCOMMANDS_CLEAR);

        hr = CreateFense(device, commands->fences + i);
        FAILED_ERROR_MESSAGE_GOTO(hr, L"fail to create dx12fence in DXCommands..", CREATEDXCOMMANDS_CLEAR);

        hr = CreateEventHandle(commands->fenceEvents + i);
        FAILED_ERROR_MESSAGE_GOTO(hr, L"fail to create win eventhandle in DXCommands..", CREATEDXCOMMANDS_CLEAR);

        commands->fenceValueSeq[i] = { 0, };
    }

    return S_OK;

CREATEDXCOMMANDS_CLEAR:

    DestroyDXCommands(commands);

    return hr;
}

HRESULT DestroyDXCommands(DXCommands* commands)
{
    if (commands->queue) {
        commands->queue->Release();
    }
    for (auto i = 0; i < CMDLIST_COUNT; i++) {
        if (commands->allocators[i]) {
            commands->allocators[i]->Release();
        }
        if (commands->commandList[i]) {
            commands->commandList[i]->Release();
        }
        if (commands->fences[i]) {
            commands->fences[i]->Release();
        }
        if (commands->fenceEvents[i]) {
            DestroyEventHandle(commands->fenceEvents[i]);
        }
    }

    return S_OK;
}