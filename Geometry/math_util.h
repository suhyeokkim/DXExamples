#pragma once

#include <cmath>
#include "defined_type.h"
#include "symbols.h"

#define INOUT
#define SQRT_OF_ONE_THIRD 0.5773502691896257645091487805019574556476f
#define EPSILON           0.001f
#define Pi2               (6.28318530717958647692f)
#define Pi                (3.14159265358979323846f)
#define InvPi             (0.31830988618379067154f)
#define Inv2Pi            (0.15915494309189533577f)
#define Inv4Pi            (0.07957747154594766788f)
#define PiOver2           (1.57079632679489661923f)
#define PiOver4           (0.78539816339744830961f)
#define Sqrt2             (1.41421356237309504880f)
#define DEG2RAD           (Pi / 180.0f)
#define RAD2DEG           (180.0f / Pi)
#define MACHINE_EPSILONE  (std::numeric_limits<float>::epsilon() * 0.5)

DECLSPEC_DLL float Lerp(float start, float end, float norm);
DECLSPEC_DLL dfloat Lerp(dfloat start, dfloat end, dfloat norm);
DECLSPEC_DLL efloat Lerp(efloat start, efloat end, efloat norm);
DECLSPEC_DLL int32 Abs(int32 val);
DECLSPEC_DLL int64 Abs(int64 val);
DECLSPEC_DLL float Abs(float val);
DECLSPEC_DLL dfloat Abs(dfloat val);
DECLSPEC_DLL efloat Abs(efloat val);
DECLSPEC_DLL int32 Clamp(int32 val, int32 min, int32 max);
DECLSPEC_DLL float Clamp(float val, float min, float max);
DECLSPEC_DLL dfloat Clamp(dfloat val, dfloat min, dfloat max);
DECLSPEC_DLL efloat Clamp(efloat val, efloat min, efloat max);
DECLSPEC_DLL float Luminance(float r, float g, float b);

struct Vector3f;
struct Vector4f;

struct DECLSPEC_DLL Vector2f
{
    union
    {
        struct
        {
            float x;
            float y;
        };
        float dataByfloat[2];
        byte  data[2 * sizeof(float)];
    };

    Vector2f();
    Vector2f(float x, float y);
    Vector2f(const Vector2f &v);

    operator Vector3f() const;
    operator Vector4f() const;

    float &operator[](uint32 i);
    const float &operator[](uint32 i) const;

    const Vector2f &operator+() const;
    Vector2f operator-() const;

    Vector2f &operator+=(const Vector2f &vec2);
    Vector2f &operator-=(const Vector2f &vec2);
    Vector2f &operator*=(const float t);
    Vector2f &operator/=(const float t);
    Vector2f &operator/=(const Vector2f &v);

    float magnitude() const;
    float sqrMagnitude() const;

    Vector2f normalized() const;
    Vector2f &normalize();

    float MaxComponent();
    int MaxDimension();
    Vector2f Permute(int x, int y);
    bool IsNan() const;

    static Vector2f Zero()
    {
        return Vector2f(0, 0);
    }

    static Vector2f One()
    {
        return Vector2f(1, 1);
    }
};

DECLSPEC_DLL Vector2f operator+(Vector2f v1, Vector2f v2);
DECLSPEC_DLL Vector2f operator-(Vector2f v1, Vector2f v2);
DECLSPEC_DLL Vector2f operator*(Vector2f v, float t);
DECLSPEC_DLL Vector2f operator*(Vector2f v1, Vector2f v2);
DECLSPEC_DLL Vector2f operator*(float t, Vector2f v);
DECLSPEC_DLL Vector2f operator/(const Vector2f &v, float t);
DECLSPEC_DLL Vector2f operator/(float t, const Vector2f &v);
DECLSPEC_DLL Vector2f operator/(const Vector2f &v1, const Vector2f &v2);
DECLSPEC_DLL bool operator==(const Vector2f &v1, const Vector2f &v2);
DECLSPEC_DLL bool operator!=(const Vector2f &v1, const Vector2f &v2);
DECLSPEC_DLL bool operator<(const Vector2f &v1, const Vector2f &v2);
DECLSPEC_DLL bool operator<=(const Vector2f &v1, const Vector2f &v2);
DECLSPEC_DLL bool operator>(const Vector2f &v1, const Vector2f &v2);
DECLSPEC_DLL bool operator>=(const Vector2f &v1, const Vector2f &v2);
DECLSPEC_DLL float Dot(const Vector2f &v1, const Vector2f &v2);
DECLSPEC_DLL Vector2f Reflect(const Vector2f &v, const Vector2f &n);
DECLSPEC_DLL const Vector2f Clamp(const Vector2f &val, const Vector2f &min, const Vector2f &max);
DECLSPEC_DLL const Vector2f Clamp(const Vector2f &val, float min, const Vector2f &max);
DECLSPEC_DLL const Vector2f Clamp(const Vector2f &val, const Vector2f &min, float max);
DECLSPEC_DLL const Vector2f Clamp(const Vector2f &val, float min, float max);
DECLSPEC_DLL Vector2f Min(const Vector2f &p1, const Vector2f &p2);
DECLSPEC_DLL Vector2f Max(const Vector2f &p1, const Vector2f &p2);
DECLSPEC_DLL Vector2f Abs(const Vector2f &v);
DECLSPEC_DLL Vector2f Sqrtv(const Vector2f &v);
DECLSPEC_DLL Vector3f ToFrameFromBasis(const Vector3f &v, const Vector3f &t, const Vector3f &b, const Vector3f &n);
DECLSPEC_DLL Vector3f ToBasisFromFrame(const Vector3f &v, const Vector3f &t, const Vector3f &b, const Vector3f &n);

struct DECLSPEC_DLL Vector3f
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
        };
        float dataByfloat[3];
        byte  data[3 * sizeof(float)];
    };

    Vector3f();
    Vector3f(float val);
    Vector3f(float x, float y, float z);
    Vector3f(const Vector2f &v, const float &o);
    Vector3f(const float &o, const Vector2f &v);
    Vector3f(const Vector3f &v);

    operator Vector2f() const;
    operator Vector4f() const;

    float &operator[](uint32 i);
    const float &operator[](uint32 i) const;

    const Vector3f &operator+() const;
    Vector3f operator-() const;

    Vector3f &operator+=(const Vector3f &vec3);
    Vector3f &operator-=(const Vector3f &vec3);
    Vector3f &operator*=(const float t);
    Vector3f &operator/=(const float t);
    Vector3f &operator/=(const Vector3f &v);
    Vector3f &operator=(const Vector3f &v);

    float magnitude() const;
    float sqrMagnitude() const;

    Vector3f normalized() const;
    Vector3f &normalize();

    static Vector3f Zero();
    static Vector3f One();

    bool IsNan() const;
    float MaxComponent();
    int MaxDimension();
    Vector3f Permute(int x, int y, int z);
};

DECLSPEC_DLL bool operator==(const Vector3f &v1, const Vector3f &v2);
DECLSPEC_DLL bool operator!=(const Vector3f &v1, const Vector3f &v2);
DECLSPEC_DLL bool operator<(const Vector3f &v1, const Vector3f &v2);
DECLSPEC_DLL bool operator<=(const Vector3f &v1, const Vector3f &v2);
DECLSPEC_DLL bool operator>(const Vector3f &v1, const Vector3f &v2);
DECLSPEC_DLL bool operator>=(const Vector3f &v1, const Vector3f &v2);
DECLSPEC_DLL Vector3f operator+(Vector3f v1, Vector3f v2);
DECLSPEC_DLL Vector3f operator+(float t, Vector3f v1);
DECLSPEC_DLL Vector3f operator+(Vector3f v1, float t);
DECLSPEC_DLL Vector3f operator-(Vector3f v1, Vector3f v2);
DECLSPEC_DLL Vector3f operator-(float t, Vector3f v1);
DECLSPEC_DLL Vector3f operator-(Vector3f v1, float t);
DECLSPEC_DLL Vector3f operator*(Vector3f v, float t);
DECLSPEC_DLL Vector3f operator*(Vector3f v1, Vector3f v2);
DECLSPEC_DLL Vector3f operator*(float t, Vector3f v);
DECLSPEC_DLL Vector3f operator/(const Vector3f &v, float t);
DECLSPEC_DLL Vector3f operator/(float t, const Vector3f &v);
DECLSPEC_DLL Vector3f operator/(const Vector3f &v1, const Vector3f &v2);
DECLSPEC_DLL float Dot(Vector3f v1, Vector3f v2);
DECLSPEC_DLL Vector3f Cross(Vector3f v1, Vector3f v2);
DECLSPEC_DLL Vector3f Reflect(const Vector3f &v, const Vector3f &n);
DECLSPEC_DLL bool Refract(const Vector3f &v, const Vector3f &n, float ni_over_nt, Vector3f &refracted);
DECLSPEC_DLL const Vector3f Clamp(const Vector3f &val, const Vector3f &min, const Vector3f &max);
DECLSPEC_DLL const Vector3f Clamp(const Vector3f &val, float min, const Vector3f &max);
DECLSPEC_DLL const Vector3f Clamp(const Vector3f &val, const Vector3f &min, float max);
DECLSPEC_DLL const Vector3f Clamp(const Vector3f &val, float min, float max);
DECLSPEC_DLL Vector3f Min(const Vector3f &p1, const Vector3f &p2);
DECLSPEC_DLL Vector3f Max(const Vector3f &p1, const Vector3f &p2);
DECLSPEC_DLL Vector3f Abs(const Vector3f &v);
DECLSPEC_DLL Vector3f Sqrtv(const Vector3f &v);

struct DECLSPEC_DLL Vector4f
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        float dataByfloat[4];
        byte  data[4 * sizeof(float)];
    };

    Vector4f();
    Vector4f(float x, float y, float z, float w);
    Vector4f(const Vector2f &v0, const Vector2f &v1);
    Vector4f(const Vector2f &v, const float &o0, const float &o1);
    Vector4f(const float &o0, const Vector2f &v, const float &o1);
    Vector4f(const float &o0, const float &o1, const Vector2f &v);
    Vector4f(const Vector3f &v, const float &o);
    Vector4f(const float &o, const Vector3f &v);
    Vector4f(const Vector4f &v);

    operator Vector2f() const;
    operator Vector3f() const;

    float &operator[](uint32 i);
    const float &operator[](uint32 i) const;

    const Vector4f &operator+() const;
    Vector4f operator-() const;

    Vector4f &operator+=(const Vector4f &vec3);
    Vector4f &operator-=(const Vector4f &vec3);
    Vector4f &operator*=(const float t);
    Vector4f &operator/=(const float t);
    Vector4f &operator/=(const Vector4f &v);
    Vector4f &operator*=(const Vector4f &v);

    float magnitude() const;
    float sqrMagnitude() const;

    Vector4f normalized() const;
    Vector4f &normalize();

    static Vector4f Zero();
    static Vector4f One();

    Vector3f GetVec3();

    int MaxDimension();
    Vector4f Permute(int x, int y, int z, int w);
    bool IsNan() const;
    float MaxComponent();
};

DECLSPEC_DLL bool operator==(const Vector4f &v1, const Vector4f &v2);
DECLSPEC_DLL bool operator!=(const Vector4f &v1, const Vector4f &v2);
DECLSPEC_DLL bool operator<(const Vector4f &v1, const Vector4f &v2);
DECLSPEC_DLL bool operator<=(const Vector4f &v1, const Vector4f &v2);
DECLSPEC_DLL bool operator>(const Vector4f &v1, const Vector4f &v2);
DECLSPEC_DLL bool operator>=(const Vector4f &v1, const Vector4f &v2);
DECLSPEC_DLL Vector4f operator+(Vector4f v1, Vector4f v2);
DECLSPEC_DLL Vector4f operator-(Vector4f v1, Vector4f v2);
DECLSPEC_DLL Vector4f operator*(Vector4f v, float t);
DECLSPEC_DLL Vector4f operator*(Vector4f v1, Vector4f v2);
DECLSPEC_DLL Vector4f operator*(float t, Vector4f v);
DECLSPEC_DLL Vector4f operator/(Vector4f v, float t);
DECLSPEC_DLL Vector4f operator/(float t, Vector4f v);
DECLSPEC_DLL Vector4f operator/(const Vector4f &v1, const Vector4f &v2);
DECLSPEC_DLL float Dot(Vector4f v1, Vector4f v2);
DECLSPEC_DLL Vector4f Reflect(const Vector4f &v, const Vector4f &n);
DECLSPEC_DLL Vector4f Clamp(const Vector4f &val, const Vector4f &min, const Vector4f &max);
DECLSPEC_DLL Vector4f Clamp(const Vector4f &val, float min, const Vector4f &max);
DECLSPEC_DLL Vector4f Clamp(const Vector4f &val, const Vector4f &min, float max);
DECLSPEC_DLL Vector4f Clamp(const Vector4f &val, float min, float max);
DECLSPEC_DLL Vector4f Min(const Vector4f &p1, const Vector4f &p2);
DECLSPEC_DLL Vector4f Max(const Vector4f &p1, const Vector4f &p2);
DECLSPEC_DLL Vector4f Abs(const Vector4f &v);
DECLSPEC_DLL Vector4f Sqrtv(const Vector4f &v);

struct Matrix4x4;

struct DECLSPEC_DLL Quaternion
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
            float w;
        };
        char data[16];
    };

    Quaternion();
    Quaternion(float _x, float _y, float _z, float _w);
    Quaternion(const Quaternion &q);

    Quaternion operator/(const float f);

    float Magnitude() const;

    Quaternion Inversed() const;
    operator Vector3f() const;
    operator Vector4f() const;

    void Rotate(Vector3f &p) const;
    void ToMatrix(Matrix4x4 &matrix) const;
    void Inverse();

    static Quaternion Identity();
    static Quaternion AngleAxis(float radian, const Vector3f &axis);

    void RotateOther(INOUT Vector3f &v);
};

DECLSPEC_DLL Quaternion operator*(const Quaternion &q1, const Quaternion &q2);
DECLSPEC_DLL Vector3f operator*(const Quaternion &q1, const Vector3f &v1);
DECLSPEC_DLL Vector3f &operator*(const Quaternion &q1, Vector3f &v1);

enum class DECLSPEC_DLL EulerAngleOrder : int
{
    OrderXYZ,
    OrderXZY,
    OrderYZX,
    OrderYXZ,
    OrderZXY,
    OrderZYX,
};

struct Matrix4x4;
DECLSPEC_DLL void EulerAngleToMatrix(const Vector3f &eulerAngle, const EulerAngleOrder order, Matrix4x4 &matirx);
DECLSPEC_DLL void EulerAngleToQuaternion(const Vector3f &eulerAngle, const EulerAngleOrder order, Quaternion &q);

// column-major matrix 4x4, https://en.wikipedia.org/wiki/Row-_and_column-major_order
struct DECLSPEC_DLL Matrix4x4
{
    union
    {
        struct
        {
            float m00;
            float m10;
            float m20;
            float m30;

            float m01;
            float m11;
            float m21;
            float m31;

            float m02;
            float m12;
            float m22;
            float m32;

            float m03;
            float m13;
            float m23;
            float m33;
        };
        struct
        {
            Vector4f c0;
            Vector4f c1;
            Vector4f c2;
            Vector4f c3;
        };
        Vector4f columns[4];
        char     data[64];
        float    dataf[16];
    };

    Matrix4x4();
    Matrix4x4(const Matrix4x4 &mat);
    Matrix4x4(const Vector4f &c0, const Vector4f &c1, const Vector4f &c2, const Vector4f &c3);

    Vector4f GetColumn(int index) const;
    Vector4f GetRow(int index) const;

    Vector3f TransformPoint(const Vector3f &pt) const;
    Vector3f TransformVector(const Vector3f &v) const;

    static Matrix4x4 GetIdentity();
    float operator[](int index);

    static Matrix4x4 FromTRS(Vector3f translate, Quaternion rotation, Vector3f scale);
    static Matrix4x4 FromTRS(Vector3f translate, Vector3f eulerAngle, EulerAngleOrder order, Vector3f scale);
};

struct DECLSPEC_DLL Ray
{
    union
    {
        struct
        {
            Vector3f origin;
            Vector3f direction;
        };
        byte data[24];
    };

    Ray();
    Ray(const Ray &r);
    Ray(const Vector3f &o, const Vector3f &d);

    Ray &operator=(const Ray &r);
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
        byte data[24];
    };

    Bounds();
    Bounds(const Bounds &b);
    Bounds(const Vector3f &c, const Vector3f &e);

    Bounds &operator=(const Bounds &v);

    bool Intersect(const Ray &r) const;
    bool Intersect(const Ray &r, float &t) const;

    void Union(const Bounds &b);

    static Bounds Union(const Bounds &b0, const Bounds &b1);
    static void Union(const Bounds &b0, const Bounds &b1, Bounds &b);
};


// TODO:: dq helper
struct DQ
{
    Quaternion real;
    Quaternion dual;
};

// TODO:: TRS helper
struct TRS
{
    Vector3f   translate;
    Quaternion rotation;
    Vector3f   scale;
};

inline void TRSToAffine(const TRS &trs, Matrix4x4 &mat)
{
    mat = Matrix4x4::FromTRS(trs.translate, trs.rotation, trs.scale);
}

inline void AffineToTRS(const Matrix4x4 &mat, TRS &trs)
{
    // TODO:: affine to trs
}

inline void TRSToDQ(const TRS &trs, DQ &dq)
{
    // TODO:: trs to dq
}

inline void DQToTRS(const DQ &dq, TRS &trs)
{
    // TODO:: dq to trs
}