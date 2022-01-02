#include <string.h>

#include "defined_macro.h"
#include "allocators.h"
#include "container.h"
#include "mempage.h"

DECLSPEC_DLL void* memAlloc(size_t size, size_t alignment, size_t alignOffset, const wchar_t* addrspace)
{
    if (addrspace == nullptr || 
        wcscmp(L"", addrspace) == 0 ||
        wcscmp(SYSTEM_NAME, addrspace) == 0) {
        return _aligned_offset_malloc(size, alignment, alignOffset);
    } else {
        auto entry = FindEntry(addrspace);

        if (entry == nullptr) {
            auto index = AddEntry(addrspace, ALLOCATOR_MIN_PAGESIZE, false);

            if (index < 0) {
                return nullptr;
            } else {
                entry = GetEntry(index);
            }
        } 

        return entry->Allocate(size, alignment, alignOffset);
    }
}

DECLSPEC_DLL bool memFree(void* ptr, const wchar_t* addrspace)
{
    if (addrspace == nullptr ||
        wcscmp(L"", addrspace) == 0 ||
        wcscmp(SYSTEM_NAME, addrspace) == 0) {
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

DECLSPEC_DLL size_t memAllocSize(const wchar_t* addrspace)
{
    auto entry = FindEntry(addrspace);

    if (entry == nullptr) {
        return 0;
    }

    return entry->allocByteCount;
}

DECLSPEC_DLL size_t memPageSize(const wchar_t* addrspace)
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

DECLSPEC_DLL size_t memPageMinSize(bool pageLocked)
{
    return pageLocked? ALLOCATOR_MIN_LOCKEDPAGESIZE: ALLOCATOR_MIN_PAGESIZE;
}

DECLSPEC_DLL bool memPageAdd(const wchar_t* addrspace, size_t pageSize, bool pageLocked)
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

DECLSPEC_DLL bool memPageFree(const wchar_t* addrspace)
{
    if (addrspace == nullptr ||
        wcscmp(L"", addrspace) == 0 ||
        wcscmp(SYSTEM_NAME, addrspace) == 0) {
        return false;
    }

    return RemoveEntry(addrspace);
}

DECLSPEC_DLL int32 validPageCount()
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
