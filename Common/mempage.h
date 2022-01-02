#pragma once

#include "defined_type.h"
#include "container.h"

constexpr int32 ALLOCATOR_MIN_ALIGNMENT = 8;
constexpr int32 ALLOCATOR_MIN_PAGESIZE = 16 * 1024 * 1024;
constexpr int32 ALLOCATOR_MIN_LOCKEDPAGESIZE = 4 * 1024;

struct MemRange
{
    size_t start;
    size_t count;
    int32 flags;
    size_t End();

    MemRange();
    MemRange(size_t start, size_t count, int flags);
};

#include <iostream>
struct MemChunk
{
    void* memPtr;
    size_t size;
    ArrayList rangeList;

    MemChunk();
    MemChunk(void* mem_ptr, size_t size);
    MemChunk(const MemChunk& o) = default;
    MemChunk(MemChunk&& o) = default;
    MemChunk& operator=(const MemChunk&) = default;
    MemChunk& operator=(MemChunk&&) = default;

    bool GetEmptyMemAndAppend(IN size_t req_size, IN size_t alignment, IN size_t aligned_offset, IN int flags, OUT void** ptr);
    bool RemoveRange(IN void* p, OUT size_t* count);
};

struct AllocatorEntry
{
    wchar_t name[256];
    unsigned debug;
    size_t allocByteCount;
    size_t minPageSize;
    size_t totalPageSize;
    size_t lastRefPage;
    ArrayList memChunkList;
    bool pageLocked;

    const size_t name_buffer_max = 256;

    AllocatorEntry();
    AllocatorEntry(const wchar_t* name, unsigned debug, size_t minPageSize, bool pageLocked);
    AllocatorEntry(const AllocatorEntry& o);
    AllocatorEntry(const AllocatorEntry& o, const wchar_t* name);
    ~AllocatorEntry();

    void* Allocate(size_t numBytes, int flags = 0);
    void* Allocate(size_t numBytes, size_t alignment, size_t offset, int flags = 0);
    bool Deallocate(void* p);
};

constexpr int ALLOCATOR_ENTRY_COUNT = 16;

AllocatorEntry* GetEntry(int index);
int32 AddEntry(const wchar_t* name, size_t min_page_size, bool pageLocked);
AllocatorEntry* FindEntry(const wchar_t* name);
bool RemoveEntry(const wchar_t* name);