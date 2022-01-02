#include "mempage.h"
#define NOMINMAX
#include <windows.h>

MemRange::MemRange() : start(0), count(0), flags(0) {}
MemRange::MemRange(size_t start, size_t count, int flags) : start(start), count(count), flags(flags) {}
size_t MemRange::End() { return start + count; }

MemChunk::MemChunk() : memPtr(nullptr), size(0)
{
    rangeList.Init(nullptr, sizeof(MemRange));
}
MemChunk::MemChunk(void* mem_ptr, size_t size) : memPtr(mem_ptr), size(size)
{
    rangeList.Init(nullptr, sizeof(MemRange));
    rangeList.ResizeMem(size / 1024 + 1);
}

#define CEIL_ALIGNED_TO(addr, align, offset) ((((addr) + (align - 1)) & ~(align - 1)) + offset)
#define FLOOR_ALIGNED_TO(addr, align, offset) (((addr) & ~(align - 1)) + offset)
bool MemChunk::GetEmptyMemAndAppend(IN size_t req_size, IN size_t alignment, IN size_t aligned_offset, IN int flags, OUT void** ptr)
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

bool MemChunk::RemoveRange(IN void* p, OUT size_t* count)
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
        }
        else {
        }
    }

    return false;
}

AllocatorEntry::AllocatorEntry() :
    name(L""), minPageSize(0), lastRefPage(0), debug(0), allocByteCount(0), totalPageSize(0), pageLocked(false)
{
    memChunkList.Init(nullptr, sizeof(MemChunk));
}
AllocatorEntry::AllocatorEntry(const wchar_t* name, unsigned debug, size_t minPageSize, bool pageLocked) :
    minPageSize(minPageSize), lastRefPage(0), debug(debug), allocByteCount(0), totalPageSize(0), pageLocked(pageLocked)
{
    wcscpy_s(this->name, name);
    memChunkList.Init(nullptr, sizeof(MemChunk));
}
AllocatorEntry::AllocatorEntry(const AllocatorEntry& o) :
    minPageSize(o.minPageSize), lastRefPage(o.lastRefPage), debug(o.debug), allocByteCount(o.allocByteCount),
    totalPageSize(o.totalPageSize), pageLocked(o.pageLocked)
{
    wcscpy_s(this->name, o.name);
    memChunkList.CopyFrom(o.memChunkList);
}
AllocatorEntry::AllocatorEntry(const AllocatorEntry& o, const wchar_t* name) :
    minPageSize(o.minPageSize), lastRefPage(o.lastRefPage), debug(o.debug), allocByteCount(o.allocByteCount),
    totalPageSize(o.totalPageSize), pageLocked(o.pageLocked)
{
    wcscpy_s(this->name, name);
    memChunkList.CopyFrom(o.memChunkList);
}
AllocatorEntry::~AllocatorEntry()
{
    for (auto i = 0; i < memChunkList.Count(); i++) {
        auto memChunk = (MemChunk*)memChunkList[i];
        if (memChunk->memPtr) {
            if (pageLocked) {
                VirtualUnlock(memChunk->memPtr, memChunk->size);
            }

            _aligned_free(memChunk->memPtr);
        }
    }
}

void* AllocatorEntry::Allocate(size_t numBytes, int flags)
{
    return Allocate(numBytes, ALLOCATOR_MIN_ALIGNMENT, 0, flags);
}
void* AllocatorEntry::Allocate(size_t numBytes, size_t alignment, size_t offset, int flags)
{
    if (memChunkList.Count() == 0) {
        size_t allocSize = numBytes < minPageSize ? minPageSize : numBytes;
        totalPageSize += allocSize;
        void* page = _aligned_offset_malloc(allocSize, minPageSize, 0);
        if (page == nullptr ||
            (pageLocked && !VirtualLock(page, allocSize))) {
            return nullptr;
        }
        auto chunk = MemChunk(page, allocSize);
        memChunkList.InsertLast(1, &chunk, nullptr);
        lastRefPage = 0;
    }

    void* p = nullptr;
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
    if (page == nullptr ||
        (pageLocked && !VirtualLock(page, allocSize))) {
        return nullptr;
    }
    auto chunk = MemChunk(page, allocSize);
    memChunkList.InsertLast(1, &chunk, nullptr);

    lastRefPage = memChunkList.Count() - 1;
    lastRefChunk = (MemChunk*)memChunkList[lastRefPage];

    if (lastRefChunk->GetEmptyMemAndAppend(numBytes, alignment, offset, flags, &p)) {
        allocByteCount += numBytes;
        return p;
    }
    else {
        return nullptr;
    }
}

bool AllocatorEntry::Deallocate(void* p)
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

AllocatorEntry g_Entries[ALLOCATOR_ENTRY_COUNT];

int32 AddEntry(const wchar_t* name, size_t min_page_size, bool pageLocked)
{
    if (name[0] == L'\0') {
        return -1;
    }

    int32 index = std::numeric_limits<int32>::max();
    for (int32 i = 0; i < ALLOCATOR_ENTRY_COUNT; i++)
        if (wcscmp(g_Entries[i].name, L"") == 0)
        {
            index = i;
            break;
        }

    if (index == std::numeric_limits<int32>::max()) return -1;

    g_Entries[index].~AllocatorEntry();
    new (g_Entries + index) AllocatorEntry(name, 1, min_page_size, pageLocked);

    return index;
}
AllocatorEntry* FindEntry(const wchar_t* name)
{
    for (int32 i = 0; i < ALLOCATOR_ENTRY_COUNT; i++)
        if (!wcscmp(g_Entries[i].name, name))
            return g_Entries + i;
    return nullptr;
}
AllocatorEntry* GetEntry(int index)
{
    return g_Entries + index;
}
bool RemoveEntry(const wchar_t* name)
{
    auto entry = FindEntry(name);

    if (entry == nullptr) {
        return false;
    }

    entry->~AllocatorEntry();
    return true;
}
