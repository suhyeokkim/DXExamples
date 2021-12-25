#include "math_util.h"
#include "catch.hpp"

TEST_CASE("test clamp", "[Math]") {
    {
        int16 min = 01, max = 10;
        int16 value0 = 00, result0 = 1;
        int16 value1 = 05, result1 = 5;
        int16 value2 = 11, result2 = 10;

        REQUIRE(Clamp(value0, min, max) == result0);
        REQUIRE(Clamp(value1, min, max) == result1);
        REQUIRE(Clamp(value2, min, max) == result2);
    }

    {
        uint16 min = 01, max = 10;
        uint16 value0 = 00, result0 = 1;
        uint16 value1 = 05, result1 = 5;
        uint16 value2 = 11, result2 = 10;

        REQUIRE(Clamp(value0, min, max) == result0);
        REQUIRE(Clamp(value1, min, max) == result1);
        REQUIRE(Clamp(value2, min, max) == result2);
    }

    {
        int32 min = 01, max = 10;
        int32 value0 = 00, result0 = 1;
        int32 value1 = 05, result1 = 5;
        int32 value2 = 11, result2 = 10;

        REQUIRE(Clamp(value0, min, max) == result0);
        REQUIRE(Clamp(value1, min, max) == result1);
        REQUIRE(Clamp(value2, min, max) == result2);
    }

    {
        uint32 min = 01, max = 10;
        uint32 value0 = 00, result0 = 1;
        uint32 value1 = 05, result1 = 5;
        uint32 value2 = 11, result2 = 10;

        REQUIRE(Clamp(value0, min, max) == result0);
        REQUIRE(Clamp(value1, min, max) == result1);
        REQUIRE(Clamp(value2, min, max) == result2);
    }

    {
        int64 min = 01, max = 10;
        int64 value0 = 00, result0 = 1;
        int64 value1 = 05, result1 = 5;
        int64 value2 = 11, result2 = 10;

        REQUIRE(Clamp(value0, min, max) == result0);
        REQUIRE(Clamp(value1, min, max) == result1);
        REQUIRE(Clamp(value2, min, max) == result2);
    }

    {
        uint64 min = 01, max = 10;
        uint64 value0 = 00, result0 = 1;
        uint64 value1 = 05, result1 = 5;
        uint64 value2 = 11, result2 = 10;

        REQUIRE(Clamp(value0, min, max) == result0);
        REQUIRE(Clamp(value1, min, max) == result1);
        REQUIRE(Clamp(value2, min, max) == result2);
    }

    {
        float min = 01, max = 10;
        float value0 = 00, result0 = 1;
        float value1 = 05, result1 = 5;
        float value2 = 11, result2 = 10;

        REQUIRE(Clamp(value0, min, max) == result0);
        REQUIRE(Clamp(value1, min, max) == result1);
        REQUIRE(Clamp(value2, min, max) == result2);
    }

    {
        dfloat min = 01, max = 10;
        dfloat value0 = 00, result0 = 1;
        dfloat value1 = 05, result1 = 5;
        dfloat value2 = 11, result2 = 10;

        REQUIRE(Clamp(value0, min, max) == result0);
        REQUIRE(Clamp(value1, min, max) == result1);
        REQUIRE(Clamp(value2, min, max) == result2);
    }

    {
        efloat min = 01, max = 10;
        efloat value0 = 00, result0 = 1;
        efloat value1 = 05, result1 = 5;
        efloat value2 = 11, result2 = 10;

        REQUIRE(Clamp(value0, min, max) == result0);
        REQUIRE(Clamp(value1, min, max) == result1);
        REQUIRE(Clamp(value2, min, max) == result2);
    }
}

TEST_CASE("test lerp", "[Math]") {
    {
        float min = 00, max = 10;
        float value0 = 0.0f, result0 = 0;
        float value1 = 0.5f, result1 = 5;
        float value2 = 1.1f, result2 = 11;

        REQUIRE(Lerp(min, max, value0) == result0);
        REQUIRE(Lerp(min, max, value1) == result1);
        REQUIRE(Lerp(min, max, value2) == result2);
    }
    {
        dfloat min = 00, max = 10;
        dfloat value0 = 0.0, result0 = 0;
        dfloat value1 = 0.5, result1 = 5;
        dfloat value2 = 1.1, result2 = 11;

        REQUIRE(Lerp(min, max, value0) == result0);
        REQUIRE(Lerp(min, max, value1) == result1);
        REQUIRE(Lerp(min, max, value2) == result2);
    }
    {
        efloat min = 00, max = 10;
        efloat value0 = 0.0l, result0 = 0;
        efloat value1 = 0.5l, result1 = 5;
        efloat value2 = 1.1l, result2 = 11;

        REQUIRE(Lerp(min, max, value0) == result0);
        REQUIRE(Lerp(min, max, value1) == result1);
        REQUIRE(Lerp(min, max, value2) == result2);
    }
}
