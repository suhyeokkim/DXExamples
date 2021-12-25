#include <string.h>

#include "defined_macro.h"
#include "allocators.h"
#include "container.h"

struct MemRange
{
    size_t start;
    size_t count;
    int32 flags;
    size_t End() { return start + count; }

    MemRange() : start(0), count(0), flags(0) {}
    MemRange(size_t start, size_t count, int flags) : start(start), count(count), flags(flags) {}
};

#include <iostream>
struct MemChunk
{
    void* memPtr;
    size_t size;
    ArrayList rangeList;

    MemChunk() : memPtr(nullptr), size(0) 
    {
        rangeList.Init(nullptr, sizeof(MemRange));
    }
    MemChunk(void* mem_ptr, size_t size) : memPtr(mem_ptr), size(size)
    {
        rangeList.Init(nullptr, sizeof(MemRange));
        rangeList.ResizeMem(size / 1024 + 1);
    }
    MemChunk(const MemChunk& o) = default;
    MemChunk(MemChunk&& o) = default;
    MemChunk& operator=(const MemChunk&) = default;
    MemChunk& operator=(MemChunk&&) = default;

#define CEIL_ALIGNED_TO(addr, align, offset) ((((addr) + (align - 1)) & ~(align - 1)) + offset)
#define FLOOR_ALIGNED_TO(addr, align, offset) (((addr) & ~(align - 1)) + offset)

    bool GetEmptyMemAndAppend(IN size_t req_size, IN size_t alignment, IN size_t aligned_offset, IN int flags, OUT void** ptr)
    {
        size_t start_point = std::numeric_limits<size_t>::max();

        if (rangeList.Count() > 0)
        {
            size_t aligned_start = 0, aligned_end = 0;
            int32 insert_index = 0;

            for (auto i = 0; i < rangeList.Count() + 1; i++)
            {
                if (i == 0)
                {
                    aligned_start = 0;
                    auto range = (MemRange*)rangeList.Start();
                    aligned_end = FLOOR_ALIGNED_TO(range->start, alignment, aligned_offset);
                }
                else if (i == rangeList.Count())
                {
                    auto range = (MemRange*)rangeList.Last();
                    aligned_start = CEIL_ALIGNED_TO(range->start + range->count, alignment, aligned_offset);
                    aligned_end = this->size;
                }
                else
                {
                    auto leftRange = (MemRange*)rangeList[i - 1];
                    auto rightRange = (MemRange*)rangeList[i];
                    aligned_start = CEIL_ALIGNED_TO(leftRange->End(), alignment, aligned_offset);
                    aligned_end = FLOOR_ALIGNED_TO(rightRange->start, alignment, aligned_offset);
                }

                if (req_size + aligned_start < aligned_end)
                {
                    start_point = aligned_start;
                    insert_index = i;
                    break;
                }
            }

            if (start_point == std::numeric_limits<size_t>::max())
                return false;

            auto r = MemRange(start_point, req_size, flags);
            rangeList.Insert(insert_index, 1, &r, nullptr);
        }
        else
        {
            start_point = 0;
            auto r = MemRange(start_point, req_size, flags);
            rangeList.InsertLast(1, &r, nullptr);
        }

        *ptr = static_cast<void*>(((char*)memPtr + start_point));

        return true;
    }

    bool RemoveRange(IN void* p, OUT size_t* count)
    {
        if (memPtr <= p && (void*)((char*)memPtr + size) > p)
        {
            auto i = 0;
            auto rangeCount = rangeList.Count();
            for (; i < rangeCount; i++) {
                auto r = (MemRange*)rangeList[i];
                if ((void*)((char*)memPtr + r->start) == p)
                    break;
            }

            if (i < rangeCount) {
                rangeList.Remove(i, 1);
                if (count) {
                    auto range = (MemRange*)rangeList[i];
                    *count = range->count;
                }
                return true;
            } else {
            }
        }

        return false;
    }
};

struct AllocatorEntry
{
    char* name;
    unsigned debug;
    size_t allocByteCount;
    size_t minPageSize;
    size_t totalPageSize;
    size_t lastRefPage;
    ArrayList memChunkList;

    const size_t name_buffer_max = 256;

    AllocatorEntry() :
        name(nullptr), minPageSize(0), lastRefPage(0), debug(0), allocByteCount(0), totalPageSize(0)
    {
        memChunkList.Init(nullptr, sizeof(MemChunk));
    }
    AllocatorEntry(const char* name, unsigned debug, size_t minPageSize) :
        minPageSize(minPageSize), lastRefPage(0), debug(debug), allocByteCount(0), totalPageSize(0)
    {
        ALLOC_SIZE_AND_STRCPY(nullptr, this->name, name_buffer_max, name);
        memChunkList.Init(nullptr, sizeof(MemChunk));
    }
    AllocatorEntry(const AllocatorEntry& o) :
        minPageSize(o.minPageSize), lastRefPage(o.lastRefPage), debug(o.debug), allocByteCount(o.allocByteCount), totalPageSize(0)
    {
        ALLOC_SIZE_AND_STRCPY(nullptr, this->name, name_buffer_max, name);
        memChunkList.CopyFrom(o.memChunkList);
    }
    AllocatorEntry(const AllocatorEntry& o, const char* name) :
        minPageSize(o.minPageSize), lastRefPage(o.lastRefPage), debug(o.debug), allocByteCount(o.allocByteCount), totalPageSize(0)
    {
        ALLOC_SIZE_AND_STRCPY(nullptr, this->name, name_buffer_max, name);
        memChunkList.CopyFrom(o.memChunkList);
    }
    ~AllocatorEntry()
    {
        for (auto i = 0; i < memChunkList.Count(); i++)
        {
            auto memChunk = (MemChunk*)memChunkList[i];
            if (memChunk->memPtr)
                _aligned_free(memChunk->memPtr);
        }
    }

    void* Allocate(size_t numBytes, int flags = 0)
    {
        return Allocate(numBytes, ALLOCATOR_MIN_ALIGNMENT, 0, flags);
    }
    void* Allocate(size_t numBytes, size_t alignment, size_t offset, int flags = 0)
    {
        if (memChunkList.Count() == 0)
        {
            size_t allocSize = numBytes < minPageSize ? minPageSize : numBytes;
            totalPageSize += allocSize;
            void* page = _aligned_offset_malloc(allocSize, minPageSize, 0);
            auto chunk = MemChunk(page, allocSize);
            memChunkList.InsertLast(1, &chunk, nullptr);
            lastRefPage = 0;
        }

        void *p = nullptr;
        auto lastRefChunk = (MemChunk*)memChunkList[lastRefPage];
        if (lastRefChunk->GetEmptyMemAndAppend(numBytes, alignment, offset, flags, &p)) {
            allocByteCount += numBytes;
            return p;
        }

        for (auto i = 0; i < memChunkList.Count(); i++) {
            auto memChunk = (MemChunk*)memChunkList[i];
            if (memChunk->GetEmptyMemAndAppend(numBytes, alignment, offset, flags, &p))
            {
                allocByteCount += numBytes;
                lastRefPage = i;
                return p;
            }
        }

        size_t allocSize = numBytes < minPageSize ? minPageSize : numBytes;
        totalPageSize += allocSize;
        void* page = _aligned_offset_malloc(allocSize, minPageSize, 0);
        auto chunk = MemChunk(page, allocSize);
        memChunkList.InsertLast(1, &chunk, nullptr);
        
        lastRefPage = memChunkList.Count() - 1;
        lastRefChunk = (MemChunk*)memChunkList[lastRefPage];

        if (lastRefChunk->GetEmptyMemAndAppend(numBytes, alignment, offset, flags, &p)) {
            allocByteCount += numBytes;
            return p;
        } else {
            return nullptr;
        }
    }

    bool Deallocate(void* p)
    {
        for (auto i = 0; i < memChunkList.Count(); i++)
        {
            auto memChunk = (MemChunk*)memChunkList[i];
            size_t count;
            if (memChunk->RemoveRange(p, &count))
            {
                allocByteCount -= count;
                return true;
            }
        }

        return false;
    }
};

const int AllocEntryCount = 16;
AllocatorEntry g_Entries[AllocEntryCount];

size_t AddEntry(const char* name, int32 min_page_size)
{
    int32 index = std::numeric_limits<int32>::max();
    for (int32 i = 0; i < AllocEntryCount; i++)
        if (g_Entries[i].name == nullptr)
        {
            index = i;
            break;
        }

    if (index == std::numeric_limits<int32>::max()) return index;

    g_Entries[index].~AllocatorEntry();
    new (g_Entries + index) AllocatorEntry(name, 1, min_page_size);

    return index;
}
AllocatorEntry* FindEntry(const char* name)
{
    for (int32 i = 0; i < AllocEntryCount; i++)
        if (g_Entries[i].name != nullptr && !strcmp(g_Entries[i].name, name))
            return g_Entries + i;
    return nullptr;
}
bool RemoveEntry(const char* name)
{
    auto entry = FindEntry(name);

    if (entry == nullptr) {
        return false;
    } 

    entry->~AllocatorEntry();
    return true;
}

const int32 g_MinPageSize = 16 * 1024 * 1024;

DECLSPEC_DLL void* memAlloc(size_t size, size_t alignment, size_t alignOffset, const char* addrspace)
{
    if (strcmp(SYSTEM_NAME, addrspace) == 0) {
        return _aligned_offset_malloc(size, alignment, alignOffset);
    } else {
        auto entry = FindEntry(addrspace);

        if (entry == nullptr) {
            auto index = AddEntry(addrspace, g_MinPageSize);

            if (index < 0) {
                return nullptr;
            } else {
                entry = g_Entries + index;
            }
        } 

        return entry->Allocate(size, alignment, alignOffset);
    }
}

DECLSPEC_DLL bool memFree(void* ptr, const char* addrspace)
{
    if (strcmp(SYSTEM_NAME, addrspace) == 0) {
        _aligned_free(ptr);
        return true;
    } else {
        auto entry = FindEntry(addrspace);

        if (entry == nullptr) {
            return false;
        }

        return entry->Deallocate(ptr);
    }
}

DECLSPEC_DLL size_t memAllocSize(const char* addrspace)
{
    auto entry = FindEntry(addrspace);

    if (entry == nullptr) {
        return false;
    }

    return entry->allocByteCount;
}

DECLSPEC_DLL size_t memPageSize(const char* addrspace)
{
    auto entry = FindEntry(addrspace);

    if (entry == nullptr) {
        return false;
    }

    return entry->totalPageSize;
}

DECLSPEC_DLL size_t memMinPageSize()
{
    return g_MinPageSize;
}

DECLSPEC_DLL int32 validPageCount()
{
    int32 count = 0;

    for (auto i = 0; i < AllocEntryCount; i++) {
        if (g_Entries[i].name != nullptr) {
            count++;
        }
    }

    return count;
}
