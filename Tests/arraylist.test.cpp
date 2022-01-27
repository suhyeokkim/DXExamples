#include "catch.hpp"
#include "common.h"

import container;

struct TestData {
    uint64 d0;
    uint64 d1;
    uint64 d2;
    uint64 d3;
};

bool Equal(TestData* s0, int offset0, const ArrayList& list, int count)
{
    auto p0 = s0 + offset0;

    if (count > 0) {
        if (memcmp(p0, list.Start(), sizeof(TestData)) != 0) {
            return false;
        }

        if (memcmp(p0 + count - 1, list.Last(), sizeof(TestData)) != 0) {
            return false;
        }
    }

    return memcmp(p0, list[0], sizeof(TestData) * count) == 0;
}

TEST_CASE("test ArrayList", "[Container.ArrayList]") {
    ArrayList list;

    REQUIRE(list.Init(
        nullptr, 
        sizeof(TestData),
        ArrayList::s_DefaultAlignment,
        1
    ));

    const int dataSize = 4;
    TestData datas[dataSize] = {
        { 0,  1,  2,  3}, 
        { 4,  5,  6,  7},
        { 8,  9, 10, 11},
        {12, 13, 14, 15}
    };

    REQUIRE(list.Insert(0, dataSize, datas, nullptr));
    REQUIRE(list.Count() == 4);
    REQUIRE(Equal(datas, 0, list, 4));

    REQUIRE(list.RemoveFirst());
    REQUIRE(list.Count() == 3);
    REQUIRE(Equal(datas, 1, list, 3));

    REQUIRE(list.RemoveLast());
    REQUIRE(list.Count() == 2);
    REQUIRE(Equal(datas, 1, list, 2));

    REQUIRE(list.InsertStart(1, datas, nullptr));
    REQUIRE(list.Count() == 3);
    REQUIRE(Equal(datas, 0, list, 3));

    REQUIRE(list.InsertLast(1, datas + 3, nullptr));
    REQUIRE(list.Count() == 4);
    REQUIRE(Equal(datas, 0, list, 4));

    REQUIRE(list.ResizeMem(2));
    REQUIRE(list.Capacity() == 2);
    REQUIRE(list.Count() == 2);
    REQUIRE(Equal(datas, 0, list, 2));

    REQUIRE(list.Destroy());

    REQUIRE(list.Init(
        L"1234",
        sizeof(TestData),
        ArrayList::s_DefaultAlignment,
        1
    ));

    REQUIRE(list.Insert(0, dataSize, datas, nullptr));
    REQUIRE(list.Count() == 4);
    REQUIRE(Equal(datas, 0, list, 4));

    REQUIRE(list.RemoveFirst());
    REQUIRE(list.Count() == 3);
    REQUIRE(Equal(datas, 1, list, 3));

    REQUIRE(list.RemoveLast());
    REQUIRE(list.Count() == 2);
    REQUIRE(Equal(datas, 1, list, 2));

    REQUIRE(list.InsertStart(1, datas, nullptr));
    REQUIRE(list.Count() == 3);
    REQUIRE(Equal(datas, 0, list, 3));

    REQUIRE(list.InsertLast(1, datas + 3, nullptr));
    REQUIRE(list.Count() == 4);
    REQUIRE(Equal(datas, 0, list, 4));

    REQUIRE(list.ResizeMem(2));
    REQUIRE(list.Capacity() == 2);
    REQUIRE(list.Count() == 2);
    REQUIRE(Equal(datas, 0, list, 2));

    REQUIRE(list.Destroy());
}
