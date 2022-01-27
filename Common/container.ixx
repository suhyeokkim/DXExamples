module;

#include "symbols.h"
#include "defined_type.h"
#include <cstdlib>
#include <cstring>
#include <cstdio>

export module container;
export import std.core;
export import allocators;

export enum ArrayList.Const {
    ARRAYLIST_DEFAULT_CAPACITY = 32,
    ARRAYLIST_DEFAULT_ALIGNMENT = 32
};

struct DebugPrintScope
{
    const wchar_t* wcs;

    DebugPrintScope(const wchar_t* wcs) : wcs(wcs)
    {
#if defined(_DEBUG)
        wprintf(L"[%s] ok\n", wcs);
#endif
    }

    ~DebugPrintScope()
    {
#if defined(_DEBUG)
        wprintf(L"[%s] off\n", wcs);
#endif
    }
};

uint64 BiggerPow(uint64 current, uint64 than)
{
    uint64 value = current;
    while (value < than)
    {
        if ((value & 0xf000000000000) > 0) {
            value = than;
            break;
        }

        value = value << 1;
    }
    return value;
}

/// <summary>
/// C 스타일 컨테이너, ABI 를 위해 템플릿 사용 금지
/// </summary>
export class ArrayList
{
public: // crud
    bool Init(
        const wchar_t* addrspace, int32 step,
        int32 alignment = s_DefaultAlignment, uint64 capacity = s_DefaultCapacity
    )
    {
        this->addrspace = addrspace != nullptr && wcscmp(addrspace, L"") != 0 ? addrspace : SYSTEM_NAME;
        this->step = step;
        this->capacity = capacity;
        this->alignment = alignment;
        this->count = 0;

        singleChunkPtr = memAlloc(step * capacity, alignment, 0, this->addrspace);
        return singleChunkPtr != nullptr;
    }

    bool Destroy()
    {
        if (singleChunkPtr != nullptr) {
            this->capacity = 0;
            memFree(singleChunkPtr, addrspace);
            singleChunkPtr = nullptr;
        }

        this->count = 0;
        this->alignment = 0;
        this->step = 0;
        this->addrspace = nullptr;

        return true;
    }

    bool CopyFrom(const ArrayList* list)
    {
        ResizeMem(list->capacity);

        this->step = list->step;
        this->alignment = list->alignment;
        this->count = list->count;

        memcpy(singleChunkPtr, list->singleChunkPtr, this->count * this->step);
        return true;
    }

    bool CopyFrom(const ArrayList& list)
    {
        ResizeMem(list.capacity);

        this->step = list.step;
        this->alignment = list.alignment;
        this->count = list.count;

        memcpy(singleChunkPtr, list.singleChunkPtr, this->count * this->step);
        return true;
    }

public:
    #define min(a, b) ((a < b)? (a): (b))

    bool ResizeMem(uint64 capacity)
    {
        auto newPtr = memAlloc((uint64)step * capacity, alignment, 0, addrspace);

        if (newPtr == nullptr) {
            return false;
        }

        if (singleChunkPtr != nullptr) {
            auto minCapacitySize = min((uint64)step * capacity, (uint64)step * this->capacity);
            memcpy(newPtr, singleChunkPtr, minCapacitySize);
        }

        this->capacity = capacity;
        singleChunkPtr = newPtr;

        if (capacity < count) {
            count = capacity;
        }

        return true;
    }
    bool ResizeMore(uint64 count)
    {
        if (count <= capacity) {
            return true;
        }

        auto newCapacity = BiggerPow(capacity, count);
        return ResizeMem(newCapacity);
    }

public:
    bool Insert(int32 startIndex, int32 count, void* ptr, void* param)
    {
        if (ptr == nullptr) {
            return false;
        }

        if (this->count + count > capacity && !ResizeMore(this->count + count)) {
            return false;
        }

        // 공간 확보 하기, 이거 없으면 Add 랑 동일
        if (startIndex + 1 < this->count) {
            auto srcArrPtr = (char*)singleChunkPtr + ((startIndex)*step);
            auto dstArrPtr = (char*)singleChunkPtr + ((startIndex + count) * step);
            auto remainItemCount = this->count - startIndex;

            // memmove -> memcpy?
            memmove(dstArrPtr, srcArrPtr, remainItemCount * step);
        }

        // 공간 확보 후 옮기기
        auto destArrPtr = (char*)singleChunkPtr + (startIndex * step);
        memcpy(destArrPtr, ptr, count * step);

        this->count = this->count + count;

        return true;
    }
    bool Remove(int32 startIndex, int32 count)
    {
        // 마지막 아니면?
        if (startIndex + count < this->count) {
            auto srcArrPtr = (char*)singleChunkPtr + ((startIndex + count) * step);
            auto dstArrPtr = (char*)singleChunkPtr + ((startIndex)*step);
            auto remainItemCount = this->count - (startIndex + count);

            // memmove -> memcpy?
            memmove(dstArrPtr, srcArrPtr, remainItemCount * step);
        }

        this->count -= count;
        return true;
    }

public: // crud helper
    bool InsertStart(int32 count, void* ptr, void* param)
    {
        return Insert(0, count, ptr, param);
    }
    bool InsertLast(int32 count, void* ptr, void* param)
    {
        return Insert(this->count, count, ptr, param);
    }
    bool RemoveFirst()
    {
        return Remove(0, 1);
    }
    bool RemoveLast()
    {
        return Remove(this->count - 1, 1);
    }
    bool RemoveAll()
    {
        this->count = 0;
        return true;
    }

public: // list accessor
    void* operator[](uint64 index) const
    {
        return (char*)singleChunkPtr + index * step;
    }
    void* Start() const
    {
        return singleChunkPtr;
    }
    void* Last() const
    {
        return (char*)singleChunkPtr + (count - 1) * step;
    }

    int32 Count() const
    {
        return count;
    }
    int32 Step() const
    {
        return step;
    }
    uint64 Capacity() const
    {
        return capacity;
    }
    int32 Alignment() const
    {
        return alignment;
    }

private:
    void* singleChunkPtr;

    const wchar_t* addrspace;
    int32 count;
    int32 step;
    uint64 capacity;
    int32 alignment;

public:
    static const uint64 s_DefaultCapacity = ARRAYLIST_DEFAULT_CAPACITY;
    static const uint32 s_DefaultAlignment = ARRAYLIST_DEFAULT_ALIGNMENT;

};