#include "allocators.h"
#include "mempage.h"
#include "defined_type.h"
#include "catch.hpp"

TEST_CASE("test memory allocate", "[Allocator]") {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);

    auto addrspace0 = L"1234";

    const int count = 4;
    uint32 sa[count] = { 12, 16, 33, 7 };
    char* sp[count] = { 0, };
    uint32 total = 0;

    for (auto i = 0; i < count; i++) {
        total += sa[i];
        sp[i] = (char*)memAlloc(sa[i], 8, 0, addrspace0);

        REQUIRE(sp[i] != nullptr);

        for (auto j = 0; j < sa[i]; j++) {
            sp[i][j] = (char)0;
        }
    }

    REQUIRE(memAllocSize(addrspace0) == total);
    REQUIRE(memPageSize(addrspace0) == memPageMinSize(false));

    REQUIRE(memFree(sp[1], addrspace0));
    sp[1] = (char*)memAlloc(sa[1], 8, 0, addrspace0);
    REQUIRE(sp[1] != nullptr);

    for (auto i = 0; i < count; i++) {
        for (auto j = 0; j < sa[i]; j++) {
            sp[i][j] = (char)0;
        }
    }

    REQUIRE(memPageFree(addrspace0));
}

TEST_CASE("test memory allocate (page locked)", "[Allocator]") {
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF);

    auto addrspace0 = L"5678";
    auto pageSize = memPageMinSize(true);

    REQUIRE(memPageAdd(addrspace0, pageSize, true));

    const int count = 4;
    uint32 sa[count] = { 12, 16, 33, 7 };
    char* sp[count] = { 0, };
    uint32 total = 0;

    for (auto i = 0; i < count; i++) {
        total += sa[i];
        sp[i] = (char*)memAlloc(sa[i], 8, 0, addrspace0);

        REQUIRE(sp[i] != nullptr);

        for (auto j = 0; j < sa[i]; j++) {
            sp[i][j] = (char)0;
        }
    }

    REQUIRE(memAllocSize(addrspace0) == total);
    REQUIRE(memPageSize(addrspace0) == memPageMinSize(true));

    REQUIRE(memFree(sp[1], addrspace0));
    sp[1] = (char*)memAlloc(sa[1], 8, 0, addrspace0);
    REQUIRE(sp[1] != nullptr);

    for (auto i = 0; i < count; i++) {
        for (auto j = 0; j < sa[i]; j++) {
            sp[i][j] = (char)0;
        }
    }
}