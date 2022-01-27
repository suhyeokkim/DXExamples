module;

#include "symbols.h"
#include "defined_type.h"

export module allocators;
export import std.core;

constexpr const wchar_t* SYSTEM_NAME = L"system";
constexpr const wchar_t* PERSISTANT_NAME = L"persitant";
constexpr const wchar_t* TEMPARARY_NAME = L"temp";

export enum Allcators.Const{
    ALLOCATOR_ENTRY_COUNT = 16,
}

AllocatorEntry g_Entries[ALLOCATOR_ENTRY_COUNT];

export
{

    export AllocatorEntry* GetEntry(int index)
    {
        return g_Entries + index;
    }

    export int32 AddEntry(const wchar_t* name, size_t min_page_size, bool pageLocked)
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

    export AllocatorEntry* FindEntry(const wchar_t* name)
    {
        for (int32 i = 0; i < ALLOCATOR_ENTRY_COUNT; i++)
            if (!wcscmp(g_Entries[i].name, name))
                return g_Entries + i;
        return nullptr;
    }

    export bool RemoveEntry(const wchar_t* name)
    {
        auto entry = FindEntry(name);

        if (entry == nullptr) {
            return false;
        }

        entry->~AllocatorEntry();
        return true;
    }

    void* memAlloc(size_t size, size_t alignment, size_t alignOffset, const wchar_t* addrspace = SYSTEM_NAME)
    {
        if (addrspace == nullptr ||
            wcscmp(L"", addrspace) == 0 ||
            wcscmp(SYSTEM_NAME, addrspace) == 0) {
            return _aligned_offset_malloc(size, alignment, alignOffset);
        }
        else {
            auto entry = FindEntry(addrspace);

            if (entry == nullptr) {
                auto index = AddEntry(addrspace, ALLOCATOR_MIN_PAGESIZE, false);

                if (index < 0) {
                    return nullptr;
                }
                else {
                    entry = GetEntry(index);
                }
            }

            return entry->Allocate(size, alignment, alignOffset);
        }
    }

    bool memFree(void* ptr, const wchar_t* addrspace = SYSTEM_NAME)
    {
        if (addrspace == nullptr ||
            wcscmp(L"", addrspace) == 0 ||
            wcscmp(SYSTEM_NAME, addrspace) == 0) {
            _aligned_free(ptr);
            return true;
        }
        else {
            auto entry = FindEntry(addrspace);

            if (entry == nullptr) {
                return false;
            }

            return entry->Deallocate(ptr);
        }
    }

    size_t memAllocSize(const wchar_t* addrspace)
    {
        auto entry = FindEntry(addrspace);

        if (entry == nullptr) {
            return 0;
        }

        return entry->allocByteCount;
    }

    size_t memPageSize(const wchar_t* addrspace)
    {
        if (addrspace == nullptr ||
            wcscmp(L"", addrspace) == 0 ||
            wcscmp(SYSTEM_NAME, addrspace) == 0) {
            return 0;
        }

        auto entry = FindEntry(addrspace);

        if (entry == nullptr) {
            return 0;
        }

        return entry->totalPageSize;
    }

    size_t memPageMinSize(bool pageLocked)
    {
        return pageLocked ? ALLOCATOR_MIN_LOCKEDPAGESIZE : ALLOCATOR_MIN_PAGESIZE;
    }

    bool memPageAdd(const wchar_t* addrspace, size_t pageSize, bool pageLocked)
    {
        if (addrspace == nullptr ||
            wcscmp(L"", addrspace) == 0 ||
            wcscmp(SYSTEM_NAME, addrspace) == 0) {
            return false;
        }

        auto entry = FindEntry(addrspace);

        if (entry != nullptr) {
            return false;
        }

        auto index = AddEntry(addrspace, pageSize, pageLocked);

        return index >= 0 && index < ALLOCATOR_ENTRY_COUNT;
    }

    bool memPageFree(const wchar_t* addrspace)
    {
        if (addrspace == nullptr ||
            wcscmp(L"", addrspace) == 0 ||
            wcscmp(SYSTEM_NAME, addrspace) == 0) {
            return false;
        }

        return RemoveEntry(addrspace);
    }

    int32 validPageCount()
    {
        int32 count = 0;

        for (auto i = 0; i < ALLOCATOR_ENTRY_COUNT; i++) {
            auto entry = GetEntry(i);
            if (wcscmp(entry->name, L"") == 0) {
                count++;
            }
        }

        return count;
    }
}