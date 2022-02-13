#pragma once

#include "defined_type.h"
#include "symbols.h"
#include "units.h"

struct DECLSPEC_DLL Ray
{
    union
    {
        struct
        {
            Vector3f origin;
            Vector3f direction;
        };
        uint8 data[24];
    };

    Ray();
    Ray(const Ray& r);
    Ray(const Vector3f& o, const Vector3f& d);

    Ray& operator=(const Ray& r);
    Vector3f GetPosAt(float t) const;
    Ray Inversed() const;
};

struct DECLSPEC_DLL Bounds
{
    union
    {
        struct
        {
            Vector3f center;
            Vector3f extents;
        };
        uint8 data[24];
    };

    Bounds();
    Bounds(const Bounds& b);
    Bounds(const Vector3f& c, const Vector3f& e);

    Bounds& operator=(const Bounds& v);

    bool Intersect(const Ray& r) const;
    bool Intersect(const Ray& r, float& t) const;

    void Union(const Bounds& b);

    static Bounds Union(const Bounds& b0, const Bounds& b1);
    static void Union(const Bounds& b0, const Bounds& b1, Bounds& b);
};

