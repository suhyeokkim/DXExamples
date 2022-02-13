#include "geometry.h"
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

TEST_CASE("test vector2", "[Math]") {
    Vector2f v2;
    REQUIRE(v2.x == 0);
    REQUIRE(v2.y == 0);
    new (&v2) Vector2f(Vector2f::One());
    REQUIRE(v2 == Vector2f(1, 1));
    new (&v2) Vector2f(Vector2f::Zero());
    REQUIRE(v2 == Vector2f(0, 0));
    new (&v2) Vector2f(2.0f, 3.0f);
    REQUIRE(v2.x == 2.0f);
    REQUIRE(v2.y == 3.0f);

    auto v3 = (Vector3f)v2;
    REQUIRE(v3.x == 2.0f);
    REQUIRE(v3.y == 3.0f);
    REQUIRE(v3.z == 0.0f);
    auto v4 = (Vector4f)v2;
    REQUIRE(v4.x == 2.0f);
    REQUIRE(v4.y == 3.0f);
    REQUIRE(v4.z == 0.0f);
    REQUIRE(v4.w == 0.0f);

    REQUIRE(v2[0] == 2.0f);
    REQUIRE(v2[1] == 3.0f);

    REQUIRE((-Vector2f::One()) * 2 + (+Vector2f::One()) == Vector2f::One() * -1);
    REQUIRE(Vector2f::One() * 2 + Vector2f::One() != Vector2f::One() * 2.5f);
    REQUIRE(Vector2f::One() * 2 + Vector2f::One() / 2 == Vector2f::One() * 2.5f);

    REQUIRE(v2.MaxDimension() == 1);
    REQUIRE(v2.MaxComponent() == 3);
    REQUIRE(v2.Permute(1, 0) == Vector2f(3.0f, 2.0f));
    REQUIRE(Vector2f(std::nanf("1"), std::nanf("2")).IsNan() == true);

    REQUIRE(v2 + Vector2f(3, 2) == Vector2f::One() * 5.f);
    REQUIRE(Vector2f(3, 2) + v2 == Vector2f::One() * 5.f);
    v2 += Vector2f(3, 2);
    REQUIRE(v2 == Vector2f::One() * 5.f);

    REQUIRE(v2 - Vector2f(2, 2) == Vector2f::One() * 3.f);
    v2 -= Vector2f(2, 2);
    REQUIRE(v2 == Vector2f::One() * 3.f);

    REQUIRE(2 * v2 == Vector2f::One() * 6.f);
    REQUIRE(v2 * 2 == Vector2f::One() * 6.f);
    v2 *= 2;
    REQUIRE(v2 == Vector2f::One() * 6.f);

    REQUIRE(v2 / 2 == Vector2f::One() * 3.f);
    v2 /= 2;
    REQUIRE(v2 == Vector2f::One() * 3.f);

    REQUIRE(v2 / Vector2f(3.f, 3.f) == Vector2f::One());
    v2 /= Vector2f(3.f, 3.f);
    REQUIRE(v2 == Vector2f::One());

    new (&v2) Vector2f(3.0f, 4.0f);

    REQUIRE(v2.magnitude() == 5.0f);
    REQUIRE(v2.sqrMagnitude() == 25.0f);
    REQUIRE(v2.normalized() == v2 / v2.magnitude());

    auto src = v2;
    v2.normalize();
    REQUIRE(v2 * src.magnitude() == src);

    REQUIRE(Vector2f::Zero() <= Vector2f::One());
    REQUIRE(Vector2f::Zero() <= Vector2f::Zero());
    REQUIRE(Vector2f::Zero() < Vector2f::One());
    REQUIRE(Vector2f::One() >= Vector2f::Zero());
    REQUIRE(Vector2f::One() >= Vector2f::One());
    REQUIRE(Vector2f::One() > Vector2f::Zero());

    REQUIRE(Dot(Vector2f(0, 1), Vector2f(1, 0)) == 0);
    REQUIRE(Dot(Vector2f(1, 0), Vector2f(1, 0)) == 1);
    REQUIRE(Abs(Dot(Vector2f(1, 1).normalized(), Vector2f(1, 0)) - cosf(Pi / 4.0f)) < EPSILON);
    REQUIRE(Min(Vector2f::Zero(), Vector2f::One()) == Vector2f::Zero());
    REQUIRE(Max(Vector2f::Zero(), Vector2f::One()) == Vector2f::One());
    REQUIRE(Clamp(-Vector2f::One(), Vector2f::Zero(), Vector2f::One()) == Vector2f::Zero());
    REQUIRE(Clamp(2 * Vector2f::One(), Vector2f::Zero(), Vector2f::One()) == Vector2f::One());
    REQUIRE(Clamp(-Vector2f::One(), Vector2f::Zero(), 1) == Vector2f::Zero());
    REQUIRE(Clamp(2 * Vector2f::One(), Vector2f::Zero(), 1) == Vector2f::One());
    REQUIRE(Clamp(-Vector2f::One(), 0, Vector2f::One()) == Vector2f::Zero());
    REQUIRE(Clamp(2 * Vector2f::One(), 0, Vector2f::One()) == Vector2f::One());
    REQUIRE(Clamp(-Vector2f::One(), 0, 1) == Vector2f::Zero());
    REQUIRE(Clamp(2 * Vector2f::One(), 0, 1) == Vector2f::One());
    REQUIRE(Sqrtv(4 * Vector2f::One()) == 2 * Vector2f::One());
    //DECLSPEC_DLL Vector2f Reflect(const Vector2f & v, const Vector2f & n);
}

TEST_CASE("test quaternion", "[Math]") {
}

TEST_CASE("test matrix", "[Math]") {
}