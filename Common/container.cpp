#include "container.h"


DECLSPEC_DLL bool ArrayList::Init(
    void(*initializer)(void*, void*), int32 step, int32 alignment, uint64 capacity
)
{
    this->initializer = initializer;
    this->step = step;
    this->capacity = capacity;
    this->alignment = alignment;
    this->count = 0;

    singleChunkPtr = _aligned_offset_malloc(step * capacity, alignment, 0);
    return singleChunkPtr != nullptr;
}

DECLSPEC_DLL bool ArrayList::Destroy()
{
    this->count = 0;

    if (singleChunkPtr != nullptr) {
        this->capacity = 0;
        _aligned_free(singleChunkPtr);
    }

    this->alignment = 0;
    this->step = 0;
    this->initializer = nullptr;

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

DECLSPEC_DLL bool ArrayList::ResizeMem(uint64 capacity)
{
    auto ptr = _aligned_offset_realloc(singleChunkPtr, (uint64)step * capacity, alignment, 0);

    if (ptr == nullptr) {
        return false;
    }

    this->capacity = capacity;
    this->singleChunkPtr = ptr;

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

    if (initializer != nullptr) {
        for (auto i = 0; i < count; i++) {
            auto itemPtr = (char*)singleChunkPtr + ((this->count + i) * step);
            initializer(itemPtr, param);
        }
    }

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

DECLSPEC_DLL int32 ArrayList::GetCount() const
{
    return count;
}

DECLSPEC_DLL int32 ArrayList::GetStep() const
{
    return step;
}

DECLSPEC_DLL uint64 ArrayList::GetCapacity() const
{
    return capacity;
}

DECLSPEC_DLL int32 ArrayList::GetAlignment() const
{
    return alignment;
}