#include "container.h"
#include "allocators.h"

// 할당자 사용 안함;
DECLSPEC_DLL bool ArrayList::Init(const wchar_t* addrspace, int32 step, int32 alignment, uint64 capacity)
{
    this->addrspace = addrspace != nullptr && wcscmp(addrspace, L"") != 0? addrspace: SYSTEM_NAME;
    this->step = step;
    this->capacity = capacity;
    this->alignment = alignment;
    this->count = 0;

    singleChunkPtr = memAlloc(step * capacity, alignment, 0, this->addrspace);
    return singleChunkPtr != nullptr;
}

DECLSPEC_DLL bool ArrayList::Destroy()
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

DECLSPEC_DLL bool ArrayList::CopyFrom(const ArrayList* list)
{
    ResizeMem(list->capacity);

    this->step = list->step;
    this->alignment = list->alignment;
    this->count = list->count;

    memcpy(singleChunkPtr, list->singleChunkPtr, this->count * this->step);
    return true;
}

DECLSPEC_DLL bool ArrayList::CopyFrom(const ArrayList& list)
{
    ResizeMem(list.capacity);

    this->step = list.step;
    this->alignment = list.alignment;
    this->count = list.count;

    memcpy(singleChunkPtr, list.singleChunkPtr, this->count * this->step);
    return true;
}

#define min(a, b) ((a < b)? (a): (b))

DECLSPEC_DLL bool ArrayList::ResizeMem(uint64 capacity)
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

DECLSPEC_DLL bool ArrayList::ResizeMore(uint64 count)
{
    if (count <= capacity) {
        return true;
    }

    auto newCapacity = BiggerPow(capacity, count);
    return ResizeMem(newCapacity);
}

DECLSPEC_DLL bool ArrayList::Insert(int32 startIndex, int32 count, NOTNULL void* ptr, NULLABLE void* param)
{
    if (ptr == nullptr) {
        return false;
    }

    if (this->count + count > capacity && !ResizeMore(this->count + count)) {
        return false;
    }

    // 공간 확보 하기, 이거 없으면 Add 랑 동일
    if (startIndex + 1 < this->count) {
        auto srcArrPtr = (char*)singleChunkPtr + ((startIndex)         * step);
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

DECLSPEC_DLL bool ArrayList::Remove(int32 startIndex, int32 count)
{
    // 마지막 아니면?
    if (startIndex + count < this->count) {
        auto srcArrPtr = (char*)singleChunkPtr + ((startIndex + count) * step);
        auto dstArrPtr = (char*)singleChunkPtr + ((startIndex)         * step);
        auto remainItemCount = this->count - (startIndex + count);

        // memmove -> memcpy?
        memmove(dstArrPtr, srcArrPtr, remainItemCount * step);
    }

    this->count -= count;
    return true;
}

bool ArrayList::RemoveAll()
{
    this->count = 0;
    return true;
}

DECLSPEC_DLL bool ArrayList::RemoveFirst()
{
    return Remove(0, 1);
}

DECLSPEC_DLL bool ArrayList::RemoveLast()
{
    return Remove(this->count - 1, 1);
}

DECLSPEC_DLL bool ArrayList::InsertStart(int32 count, NOTNULL void* ptr, NULLABLE void* param)
{
    return Insert(0, count, ptr, param);
}

DECLSPEC_DLL bool ArrayList::InsertLast(int32 count, NOTNULL void* ptr, NULLABLE void* param)
{
    return Insert(this->count, count, ptr, param);
}

DECLSPEC_DLL void* ArrayList::operator[](uint64 index) const
{
    return (char*)singleChunkPtr + index * step;
}

DECLSPEC_DLL void* ArrayList::Start() const
{
    return singleChunkPtr;
}

DECLSPEC_DLL void* ArrayList::Last() const
{
    return (char*)singleChunkPtr + (count - 1) * step;
}

DECLSPEC_DLL int32 ArrayList::Count() const
{
    return count;
}

DECLSPEC_DLL int32 ArrayList::Step() const
{
    return step;
}

DECLSPEC_DLL uint64 ArrayList::Capacity() const
{
    return capacity;
}

DECLSPEC_DLL int32 ArrayList::Alignment() const
{
    return alignment;
}