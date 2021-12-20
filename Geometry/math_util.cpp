#include "math_util.h"

#pragma region POD Common

float Lerp(float start, float end, float norm)
{
    return start * (1.f - norm) + end * norm;
}

dfloat Lerp(dfloat start, dfloat end, dfloat norm)
{
    return start * (1. - norm) + end * norm;
}

efloat Lerp(efloat start, efloat end, efloat norm)
{
    return start * (1.L - norm) + end * norm;
}

int32 Abs(int32 val)
{
    if (val < 0)
        return -val;
    else
        return val;
}

int64 Abs(int64 val)
{
    if (val < 0)
        return -val;
    else
        return val;
}

float Abs(float val)
{
    return fabs(val);
}

dfloat Abs(dfloat val)
{
    return fabs(val);
}

efloat Abs(efloat val)
{
    return fabs(val);
}

int32 Clamp(int32 val, int32 min, int32 max)
{
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}

uint32 Clamp(uint32 val, uint32 min, uint32 max)
{
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}

int64 Clamp(int64 val, int64 min, int64 max)
{
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}

uint64 Clamp(uint64 val, uint64 min, uint64 max)
{
    if (val < min)
        return min;
    else if (val > max)
        return max;
    else
        return val;
}

float Clamp(float val, float min, float max)
{
    return fmax(min, fmin(max, val));
}

dfloat Clamp(dfloat val, dfloat min, dfloat max)
{
    return fmax(min, fmin(max, val));
}

efloat Clamp(efloat val, efloat min, efloat max)
{
    return fmax(min, fmin(max, val));
}

float RandomFloat(float min, float max)
{
    // this  function assumes max > min, you may want
    // more robust error checking for a non-debug build
    float random = ((float) rand()) / (float) RAND_MAX;
    
    // generate (in your case) a float between 0 and (4.5-.78)
    // then add .78, giving you a float between .78 and 4.5
    float range = max - min;
    return (random * range) + min;
}

float Luminance(float r, float g, float b)
{
    return 0.212671f * r + 0.715160f * g + 0.072169f * b;
}

Vector3f ToFrameFromBasis(const Vector3f &v, const Vector3f &t, const Vector3f &b, const Vector3f &n)
{
    return Vector3f(Dot(v, t), Dot(v, b), Dot(v, n));
}

Vector3f ToBasisFromFrame(const Vector3f &v, const Vector3f &t, const Vector3f &b, const Vector3f &n)
{
    return Vector3f(
            t.x * v.x + b.x * v.y + n.x * v.z,
            t.y * v.x + b.y * v.y + n.y * v.z,
            t.z * v.x + b.z * v.y + n.z * v.z
    );
}

#pragma endregion

#pragma region Vector2f

Vector2f::Vector2f() : x(0), y(0)
{ }

Vector2f::Vector2f(float x, float y) : x(x), y(y)
{ }

Vector2f::Vector2f(const Vector2f &v) : x(v.x), y(v.y)
{ }

float &Vector2f::operator[](uint32 i)
{
    return dataByfloat[i & 0x01];
}

const float &Vector2f::operator[](uint32 i) const
{
    return dataByfloat[i & 0x01];
}

const Vector2f &Vector2f::operator+() const
{
    return *this;
}

Vector2f Vector2f::operator-() const
{
    return Vector2f(-x, -y);
}

float Vector2f::magnitude() const
{
    return sqrt(x * x + y * y);
}

float Vector2f::sqrMagnitude() const
{
    return x * x + y * y;
}

Vector2f &Vector2f::operator+=(const Vector2f &vec2)
{
    this->x += vec2.x;
    this->y += vec2.y;
    return *this;
}

Vector2f &Vector2f::operator-=(const Vector2f &vec2)
{
    this->x -= vec2.x;
    this->y -= vec2.y;
    return *this;
}

Vector2f &Vector2f::operator*=(const float t)
{
    this->x *= t;
    this->y *= t;
    return *this;
}

Vector2f &Vector2f::operator/=(const float t)
{
    this->x /= t;
    this->y /= t;
    return *this;
}

Vector2f &Vector2f::operator/=(const Vector2f &v)
{
    this->x /= v.x;
    this->y /= v.y;
    return *this;
}

Vector2f Vector2f::normalized() const
{
    return *this / magnitude();
}

Vector2f &Vector2f::normalize()
{
    return *this /= magnitude();
}

int Vector2f::MaxDimension()
{
    return (x > y) ? 0 : 1;
}

float Vector2f::MaxComponent()
{
    return fmax(x, y);
}

Vector2f Vector2f::Permute(int x, int y)
{
    return Vector2f(dataByfloat[x], dataByfloat[y]);
}

Vector2f operator+(Vector2f v1, Vector2f v2)
{
    return Vector2f(v1.x + v2.x, v1.y + v2.y);
}

Vector2f operator-(Vector2f v1, Vector2f v2)
{
    return Vector2f(v1.x - v2.x, v1.y - v2.y);
}

Vector2f operator*(Vector2f v, float t)
{
    return Vector2f(v.x * t, v.y * t);
}

Vector2f operator*(Vector2f v1, Vector2f v2)
{
    return Vector2f(v1.x * v2.x, v1.y * v2.y);
}

Vector2f operator*(float t, Vector2f v)
{
    return Vector2f(v.x * t, v.y * t);
}

Vector2f operator/(const Vector2f &v, float t)
{
    return Vector2f(v.x / t, v.y / t);
}

Vector2f operator/(float t, const Vector2f &v)
{
    return Vector2f(t / v.x, t / v.y);
}

Vector2f operator/(const Vector2f &v1, const Vector2f &v2)
{
    return Vector2f(v1.x / v2.x, v1.y / v2.y);
}

bool operator==(const Vector2f &v1, const Vector2f &v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

bool operator!=(const Vector2f &v1, const Vector2f &v2)
{
    return v1.x != v2.x || v1.y != v2.y;
}

bool operator<(const Vector2f &v1, const Vector2f &v2)
{
    return v1.x < v2.x && v1.y < v2.y;
}

bool operator<=(const Vector2f &v1, const Vector2f &v2)
{
    return v1.x <= v2.x && v1.y <= v2.y;
}

bool operator>(const Vector2f &v1, const Vector2f &v2)
{
    return v1.x > v2.x && v1.y > v2.y;
}

bool operator>=(const Vector2f &v1, const Vector2f &v2)
{
    return v1.x >= v2.x && v1.y >= v2.y;
}

float Dot(const Vector2f &v1, const Vector2f &v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

Vector2f Reflect(const Vector2f &v, const Vector2f &n)
{
    return v - 2 * Dot(v, n) * n;
}

const Vector2f Clamp(const Vector2f &val, const Vector2f &min, const Vector2f &max)
{
    return Vector2f(Clamp(val.x, min.x, max.x), Clamp(val.y, min.y, max.y));
}

const Vector2f Clamp(const Vector2f &val, float min, const Vector2f &max)
{
    return Vector2f(Clamp(val.x, min, max.x), Clamp(val.y, min, max.y));
}

const Vector2f Clamp(const Vector2f &val, const Vector2f &min, float max)
{
    return Vector2f(Clamp(val.x, min.x, max), Clamp(val.y, min.y, max));
}

const Vector2f Clamp(const Vector2f &val, float min, float max)
{
    return Vector2f(Clamp(val.x, min, max), Clamp(val.y, min, max));
}

Vector2f Min(const Vector2f &p1, const Vector2f &p2)
{
    return Vector2f(fmin(p1.x, p2.x), fmin(p1.y, p2.y));
}

Vector2f Max(const Vector2f &p1, const Vector2f &p2)
{
    return Vector2f(fmax(p1.x, p2.x), fmax(p1.y, p2.y));
}

Vector2f Abs(const Vector2f &v)
{
    return Vector2f(std::abs(v.x), std::abs(v.y));
}

Vector2f Sqrtv(const Vector2f &v)
{
    return Vector2f(std::sqrt(v.x), std::sqrt(v.y));
}

bool Vector2f::IsNan() const
{
    return isnan(x) || isnan(y);
}

Vector2f::operator Vector3f() const
{
    return Vector3f(this->x, this->y, 0);
}

Vector2f::operator Vector4f() const
{
    return Vector4f(this->x, this->y, 0, 0);
}

#pragma endregion

#pragma region Vector3f

Vector3f::Vector3f() : x(0), y(0), z(0)
{ }

Vector3f::Vector3f(float val) : x(val), y(val), z(val)
{ }

Vector3f::Vector3f(float x, float y, float z) : x(x), y(y), z(z)
{ }

Vector3f::Vector3f(const Vector2f &v, const float &o) : x(v.x), y(v.y), z(o)
{ }

Vector3f::Vector3f(const float &o, const Vector2f &v) : x(o), y(v.x), z(v.y)
{ }

Vector3f::Vector3f(const Vector3f &v) : x(v.x), y(v.y), z(v.z)
{ }

Vector3f::operator Vector2f() const
{
    return Vector2f(this->x, this->y);
}

Vector3f::operator Vector4f() const
{
    return Vector4f(this->x, this->y, this->z, 0);
}

float &Vector3f::operator[](uint32 i)
{
    return dataByfloat[i % 3];
}

const float &Vector3f::operator[](uint32 i) const
{
    return dataByfloat[i % 3];
}

const Vector3f &Vector3f::operator+() const
{
    return *this;
}

Vector3f Vector3f::operator-() const
{
    return Vector3f(-x, -y, -z);
}

Vector3f &Vector3f::operator+=(const Vector3f &vec3)
{
    this->x += vec3.x;
    this->y += vec3.y;
    this->z += vec3.z;
    return *this;
}

Vector3f &Vector3f::operator-=(const Vector3f &vec3)
{
    this->x -= vec3.x;
    this->y -= vec3.y;
    this->z -= vec3.z;
    return *this;
}

Vector3f &Vector3f::operator*=(const float t)
{
    this->x *= t;
    this->y *= t;
    this->z *= t;
    return *this;
}

Vector3f &Vector3f::operator/=(const float t)
{
    this->x /= t;
    this->y /= t;
    this->z /= t;
    return *this;
}

Vector3f &Vector3f::operator/=(const Vector3f &v)
{
    this->x /= v.x;
    this->y /= v.y;
    this->z /= v.z;
    return *this;
}

Vector3f &Vector3f::operator=(const Vector3f &v)
{
    this->x = v.x;
    this->y = v.y;
    this->z = v.z;
    return *this;
}

float Vector3f::magnitude() const
{
    return sqrt(x * x + y * y + z * z);
}

float Vector3f::sqrMagnitude() const
{
    return x * x + y * y + z * z;
}

Vector3f Vector3f::normalized() const
{
    float mag = magnitude();
    return Vector3f(x / mag, y / mag, z / mag);
}

Vector3f &Vector3f::normalize()
{
    return *this /= magnitude();
}

Vector3f Vector3f::Zero()
{
    return Vector3f(0, 0, 0);
}

Vector3f Vector3f::One()
{
    return Vector3f(1, 1, 1);
}

bool Vector3f::IsNan() const
{
    return isnan(x) || isnan(y) || isnan(z);
}

float Vector3f::MaxComponent()
{
    return fmax(x, fmax(y, z));
}

int Vector3f::MaxDimension()
{
    return (x > y) ? ((x > z) ? 0 : 2) : ((y > z) ? 1 : 2);
}

Vector3f Vector3f::Permute(int x, int y, int z)
{
    return Vector3f((*this)[x], (*this)[y], (*this)[z]);
}

bool operator==(const Vector3f &v1, const Vector3f &v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

bool operator!=(const Vector3f &v1, const Vector3f &v2)
{
    return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z;
}

bool operator<(const Vector3f &v1, const Vector3f &v2)
{
    return v1.x < v2.x && v1.y < v2.y && v1.z < v2.z;
}

bool operator<=(const Vector3f &v1, const Vector3f &v2)
{
    return v1.x <= v2.x && v1.y <= v2.y && v1.z <= v2.z;
}

bool operator>(const Vector3f &v1, const Vector3f &v2)
{
    return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z;
}

bool operator>=(const Vector3f &v1, const Vector3f &v2)
{
    return v1.x >= v2.x && v1.y >= v2.y && v1.z >= v2.z;
}

Vector3f operator+(Vector3f v1, Vector3f v2)
{
    return Vector3f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vector3f operator+(float t, Vector3f v1)
{
    return Vector3f(v1.x + t, v1.y + t, v1.z + t);
}

Vector3f operator+(Vector3f v1, float t)
{
    return Vector3f(v1.x + t, v1.y + t, v1.z + t);
}

Vector3f operator-(Vector3f v1, Vector3f v2)
{
    return Vector3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vector3f operator-(float t, Vector3f v1)
{
    return Vector3f(t - v1.x, t - v1.y, t - v1.z);
}

Vector3f operator-(Vector3f v1, float t)
{
    return Vector3f(v1.x - t, v1.y - t, v1.z - t);
}

Vector3f operator*(Vector3f v, float t)
{
    return Vector3f(v.x * t, v.y * t, v.z * t);
}

Vector3f operator*(Vector3f v1, Vector3f v2)
{
    return Vector3f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

Vector3f operator*(float t, Vector3f v)
{
    return Vector3f(v.x * t, v.y * t, v.z * t);
}

Vector3f operator/(const Vector3f &v, float t)
{
    return Vector3f(v.x / t, v.y / t, v.z / t);
}

Vector3f operator/(float t, const Vector3f &v)
{
    return Vector3f(t / v.x, t / v.y, t / v.z);
}

Vector3f operator/(const Vector3f &v1, const Vector3f &v2)
{
    return Vector3f(v1.x / v2.x, v1.x / v2.y, v1.z / v2.z);
}

float Dot(Vector3f v1, Vector3f v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3f Cross(Vector3f v1, Vector3f v2)
{
    return Vector3f(
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x
    );
}

Vector3f Reflect(const Vector3f &v, const Vector3f &n)
{
    return v - 2 * Dot(v, n) * n;
}

bool Refract(const Vector3f &v, const Vector3f &n, float ni_over_nt, Vector3f &refracted)
{
    Vector3f uv           = v.normalized();
    float    dt           = Dot(uv, n);
    float    discriminant = 1.f - ni_over_nt * ni_over_nt * (1 - dt * dt);
    if (discriminant > 0) {
        refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
        return true;
    } else
        return false;
}

const Vector3f Clamp(const Vector3f &val, const Vector3f &min, const Vector3f &max)
{
    return Vector3f(Clamp(val.x, min.x, max.x), Clamp(val.y, min.y, max.y), Clamp(val.z, min.z, max.z));
}

const Vector3f Clamp(const Vector3f &val, float min, const Vector3f &max)
{
    return Vector3f(Clamp(val.x, min, max.x), Clamp(val.y, min, max.y), Clamp(val.z, min, max.z));
}

const Vector3f Clamp(const Vector3f &val, const Vector3f &min, float max)
{
    return Vector3f(Clamp(val.x, min.x, max), Clamp(val.y, min.y, max), Clamp(val.z, min.z, max));
}

const Vector3f Clamp(const Vector3f &val, float min, float max)
{
    return Vector3f(Clamp(val.x, min, max), Clamp(val.y, min, max), Clamp(val.z, min, max));
}

Vector3f Min(const Vector3f &p1, const Vector3f &p2)
{
    return Vector3f(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
}

Vector3f Max(const Vector3f &p1, const Vector3f &p2)
{
    return Vector3f(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
}

Vector3f Abs(const Vector3f &v)
{
    return Vector3f(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

Vector3f Sqrtv(const Vector3f &v)
{
    return Vector3f(std::sqrt(v.x), std::sqrt(v.y), std::sqrt(v.z));
}

#pragma endregion

#pragma region Vector4f

Vector4f::Vector4f() : x(0), y(0), z(0), w(0)
{ }

Vector4f::Vector4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{ }

Vector4f::Vector4f(const Vector2f &v0, const Vector2f &v1) : x(v0.x), y(v0.y), z(v1.x), w(v1.y)
{ }

Vector4f::Vector4f(const Vector2f &v, const float &o0, const float &o1) : x(v.x), y(v.y), z(o0), w(o1)
{ }

Vector4f::Vector4f(const float &o0, const Vector2f &v, const float &o1) : x(o0), y(v.x), z(v.y), w(o1)
{ }

Vector4f::Vector4f(const float &o0, const float &o1, const Vector2f &v) : x(o0), y(o1), z(v.x), w(v.y)
{ }

Vector4f::Vector4f(const Vector3f &v, const float &o) : x(v.x), y(v.y), z(v.z), w(o)
{ }

Vector4f::Vector4f(const float &o, const Vector3f &v) : x(o), y(v.x), z(v.y), w(v.z)
{ }

Vector4f::Vector4f(const Vector4f &v) : x(v.x), y(v.y), z(v.z), w(v.w)
{ }

Vector4f::operator Vector2f() const
{
    return Vector2f(this->x, this->y);
}

Vector4f::operator Vector3f() const
{
    return Vector3f(this->x, this->y, this->z);
}

float &Vector4f::operator[](uint32 i)
{
    return dataByfloat[i & 0x03];
}

const float &Vector4f::operator[](uint32 i) const
{
    return dataByfloat[i & 0x03];
}

const Vector4f &Vector4f::operator+() const
{ return *this; }

Vector4f Vector4f::operator-() const
{ return Vector4f(-x, -y, -z, -w); }

Vector4f &Vector4f::operator+=(const Vector4f &vec3)
{
    this->x += vec3.x;
    this->y += vec3.y;
    this->z += vec3.z;
    this->w += vec3.w;
    return *this;
}

Vector4f &Vector4f::operator-=(const Vector4f &vec3)
{
    this->x -= vec3.x;
    this->y -= vec3.y;
    this->z -= vec3.z;
    this->w -= vec3.w;
    return *this;
}

Vector4f &Vector4f::operator*=(const float t)
{
    this->x *= t;
    this->y *= t;
    this->z *= t;
    this->w *= t;
    return *this;
}

Vector4f &Vector4f::operator/=(const float t)
{
    this->x /= t;
    this->y /= t;
    this->z /= t;
    this->w /= t;
    return *this;
}

Vector4f &Vector4f::operator/=(const Vector4f &v)
{
    this->x /= v.x;
    this->y /= v.y;
    this->z /= v.z;
    this->w /= v.w;
    return *this;
}

Vector4f &Vector4f::operator*=(const Vector4f &v)
{
    this->x *= v.x;
    this->y *= v.y;
    this->z *= v.z;
    this->w *= v.w;
    return *this;
}

float Vector4f::magnitude() const
{ return sqrt(x * x + y * y + z * z + w * w); }

float Vector4f::sqrMagnitude() const
{ return x * x + y * y + z * z + w * w; }

Vector4f Vector4f::normalized() const
{
    return *this / magnitude();
}

Vector4f &Vector4f::normalize()
{
    return *this /= magnitude();
}

Vector4f Vector4f::Zero()
{ return Vector4f(0, 0, 0, 0); }

Vector4f Vector4f::One()
{ return Vector4f(1, 1, 1, 1); }

Vector3f Vector4f::GetVec3()
{
    return Vector3f(x, y, z);
}

int Vector4f::MaxDimension()
{
    auto xyPtr   = (x > y) ? &x : &y;
    auto zwPtr   = (z > w) ? &z : &w;
    auto lastPtr = (*xyPtr > *zwPtr) ? xyPtr : zwPtr;
    auto diff    = lastPtr - reinterpret_cast<const float *>(this);
    
    return (int) (diff / sizeof(float));
}

Vector4f Vector4f::Permute(int x, int y, int z, int w)
{
    return Vector4f((*this)[x], (*this)[y], (*this)[z], (*this)[w]);
}

bool Vector4f::IsNan() const
{
    return isnan(x) || isnan(y) || isnan(z) || isnan(w);
}

float Vector4f::MaxComponent()
{
    return fmax(fmax(x, y), fmax(z, w));
}

bool operator==(const Vector4f &v1, const Vector4f &v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

bool operator!=(const Vector4f &v1, const Vector4f &v2)
{
    return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z || v1.w != v2.w;
}

bool operator<(const Vector4f &v1, const Vector4f &v2)
{
    return v1.x < v2.x && v1.y < v2.y && v1.z < v2.z && v1.w < v2.w;
}

bool operator<=(const Vector4f &v1, const Vector4f &v2)
{
    return v1.x <= v2.x && v1.y <= v2.y && v1.z <= v2.z && v1.w <= v2.w;
}

bool operator>(const Vector4f &v1, const Vector4f &v2)
{
    return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z && v1.w > v2.w;
}

bool operator>=(const Vector4f &v1, const Vector4f &v2)
{
    return v1.x >= v2.x && v1.y >= v2.y && v1.z >= v2.z && v1.w >= v2.w;
}

Vector4f operator+(Vector4f v1, Vector4f v2)
{
    return Vector4f(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

Vector4f operator-(Vector4f v1, Vector4f v2)
{
    return Vector4f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.z - v2.z);
}

Vector4f operator*(Vector4f v, float t)
{
    return Vector4f(v.x * t, v.y * t, v.z * t, v.w * t);
}

Vector4f operator*(Vector4f v1, Vector4f v2)
{
    return Vector4f(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

Vector4f operator*(float t, Vector4f v)
{
    return Vector4f(v.x * t, v.y * t, v.z * t, v.w * t);
}

Vector4f operator/(Vector4f v, float t)
{
    return Vector4f(v.x / t, v.y / t, v.z / t, v.w / t);
}

Vector4f operator/(float t, Vector4f v)
{
    return Vector4f(t / v.x, t / v.y, t / v.z, t / v.w);
}

Vector4f operator/(const Vector4f &v1, const Vector4f &v2)
{
    return Vector4f(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

float Dot(Vector4f v1, Vector4f v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

Vector4f Reflect(const Vector4f &v, const Vector4f &n)
{
    return v - 2 * Dot(v, n) * n;
}

Vector4f Clamp(const Vector4f &val, const Vector4f &min, const Vector4f &max)
{
    return Vector4f(
            Clamp(val.x, min.x, max.x), Clamp(val.y, min.y, max.y),
            Clamp(val.z, min.z, max.z), Clamp(val.w, min.w, max.w)
    );
}

Vector4f Clamp(const Vector4f &val, float min, const Vector4f &max)
{
    return Vector4f(
            Clamp(val.x, min, max.x), Clamp(val.y, min, max.y),
            Clamp(val.z, min, max.z), Clamp(val.w, min, max.w)
    );
}

Vector4f Clamp(const Vector4f &val, const Vector4f &min, float max)
{
    return Vector4f(
            Clamp(val.x, min.x, max), Clamp(val.y, min.y, max),
            Clamp(val.z, min.z, max), Clamp(val.w, min.w, max)
    );
}

Vector4f Clamp(const Vector4f &val, float min, float max)
{
    return Vector4f(
            Clamp(val.x, min, max), Clamp(val.y, min, max),
            Clamp(val.z, min, max), Clamp(val.w, min, max)
    );
}

Vector4f Min(const Vector4f &p1, const Vector4f &p2)
{
    return Vector4f(
            fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z), fmin(p1.w, p2.w)
    );
}

Vector4f Max(const Vector4f &p1, const Vector4f &p2)
{
    return Vector4f(
            fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.x, p2.x), fmax(p1.y, p2.y)
    );
}

Vector4f Abs(const Vector4f &v)
{
    return Vector4f(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w));
}

Vector4f Sqrtv(const Vector4f &v)
{
    return Vector4f(std::sqrt(v.x), std::sqrt(v.y), std::sqrt(v.z), std::sqrt(v.w));
}

#pragma endregion

#pragma region Quaternion

Quaternion::Quaternion()
{ }

Quaternion::Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
{ }

Quaternion::Quaternion(const Quaternion &q) : x(q.x), y(q.y), z(q.z), w(q.w)
{ }

Quaternion Quaternion::operator/(const float f)
{
    return Quaternion(x / f, y / f, z / f, w / f);
}

float Quaternion::Magnitude() const
{
    return sqrt(x * x + y * y + z * z + w * w);
}

Quaternion Quaternion::Inversed() const
{
    return Quaternion(-x, -y, -z, w);
}

Quaternion::operator Vector3f() const
{
    return Vector3f(x, y, z);
}

Quaternion::operator Vector4f() const
{
    return Vector4f(x, y, z, w);
}

void Quaternion::Inverse()
{
    x *= -1;
    y *= -1;
    z *= -1;
}

Quaternion Quaternion::Identity()
{ return Quaternion(0, 0, 0, 1); }

Quaternion Quaternion::AngleAxis(float radian, const Vector3f &axis)
{
    if (axis.IsNan() || axis == Vector3f::Zero()) return Quaternion::Identity();
    
    float sin = sinf(radian * DEG2RAD / 2.f);
    return Quaternion(sin * axis.x, sin * axis.y, sin * axis.z, cosf(radian / 2.f * DEG2RAD));
}

void Quaternion::RotateOther(INOUT Vector3f &v)
{
    // implication : quaternion is normalized.
    Quaternion p(v.x, v.y, v.z, 0), q2(-this->x, -this->y, -this->z, this->w), q;
    
    q = *this * p;
    q = q * q2;
    v.x = q.x;
    v.y = q.y;
    v.z = q.z;
}

void Quaternion::Rotate(Vector3f &p) const
{
    float num01 = x * 2, num02 = y * 2, num03 = z * 2;
    float num04 = x * num01, num05 = y * num02, num06 = z * num03;
    float num07 = x * num02, num08 = x * num03, num09 = y * num03;
    float num10 = w * num01, num11 = w * num02, num12 = w * num03;
    
    Vector3f result;
    result.x = (1 - (num05 + num06)) * p.x + (num07 - num12) * p.y + (num08 + num11) * p.z;
    result.y = (num07 + num12) * p.x + (1 - (num04 + num06)) * p.y + (num09 - num10) * p.z;
    result.z = (num08 - num11) * p.x + (num09 + num10) * p.y + (1 - (num04 + num05)) * p.z;
    p = result;
}

Quaternion operator*(const Quaternion &q1, const Quaternion &q2)
{
    return Quaternion(
            q2.w * q1.x + q2.x * q1.w + q2.y * q1.z - q2.z * q1.y,
            q2.w * q1.y + q2.y * q1.w + q2.z * q1.x - q2.x * q1.z,
            q2.w * q1.z + q2.z * q1.w + q2.x * q1.y - q2.y * q1.x,
            q2.w * q1.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z);
}

Vector3f operator*(const Quaternion &q1, const Vector3f &v1)
{
    Vector3f v = v1;
    q1.Rotate(v);
    return v;
}

Vector3f &operator*(const Quaternion &q1, Vector3f &v1)
{
    q1.Rotate(v1);
    return v1;
}

// https://neoplanetz.tistory.com/entry/CV-%EC%BF%BC%ED%84%B0%EB%8B%88%EC%96%B8%EC%9D%84-%EB%A1%9C%ED%85%8C%EC%9D%B4%EC%85%98-%EB%A7%A4%ED%8A%B8%EB%A6%AD%EC%8A%A4%EB%A1%9C-%EB%B3%80%ED%99%98-Quaternion-to-Rotation-Matrix
void Quaternion::ToMatrix(Matrix4x4 &matrix) const
{
    float xx = x * x, yy = y * y, zz = z * z;
    float xy = x * y, yz = y * z, zx = z * x;
    float xw = x * w, yw = y * w, zw = z * w;
    
    matrix.dataf[0] = 1 - 2 * (yy + zz);
    matrix.dataf[4] = 2 * (xy - zw);
    matrix.dataf[8] = 2 * (zx + yw);
    matrix.dataf[3] = 0;
    
    matrix.dataf[1] = 2 * (xy + zw);
    matrix.dataf[5] = 1 - 2 * (xx + zz);
    matrix.dataf[9] = 2 * (yz - xw);
    matrix.dataf[7] = 0;
    
    matrix.dataf[2]  = 2 * (zx - yw);
    matrix.dataf[6]  = 2 * (yz + xw);
    matrix.dataf[10] = 1 - 2 * (xx + yy);
    matrix.dataf[11] = 0;
    
    matrix.dataf[12] = 0;
    matrix.dataf[13] = 0;
    matrix.dataf[14] = 0;
    matrix.dataf[15] = 1;
}


#pragma endregion

#pragma region Matrix4x4

Matrix4x4::Matrix4x4()
{
    m00 = m10 = m20 = m30 =
    m01 = m11 = m12 = m13 =
    m02 = m12 = m22 = m32 =
    m03 = m13 = m23 = m33 = 0;
}

Matrix4x4::Matrix4x4(const Matrix4x4 &mat) :
        m00(mat.m00), m10(mat.m10), m20(mat.m20), m30(mat.m30),
        m01(mat.m01), m11(mat.m11), m21(mat.m21), m31(mat.m31),
        m02(mat.m02), m12(mat.m12), m22(mat.m22), m32(mat.m32),
        m03(mat.m03), m13(mat.m13), m23(mat.m23), m33(mat.m33)
{ }

Matrix4x4::Matrix4x4(const Vector4f &c0, const Vector4f &c1, const Vector4f &c2, const Vector4f &c3)
{
    this->c0 = c0;
    this->c1 = c1;
    this->c2 = c2;
    this->c3 = c3;
}

Vector4f Matrix4x4::GetColumn(int index) const
{
    return columns[index];
}

Vector4f Matrix4x4::GetRow(int index) const
{
    return Vector4f(
            dataf[0 * 4 + index], dataf[1 * 4 + index],
            dataf[2 * 4 + index], dataf[3 * 4 + index]
    );
}

Vector3f Matrix4x4::TransformPoint(const Vector3f &pt) const
{
    Vector4f result;
    
    result.x = m00 * pt.x;
    result.y = m10 * pt.x;
    result.z = m20 * pt.x;
    result.w = m30 * pt.x;
    result.x += m01 * pt.y;
    result.y += m11 * pt.y;
    result.z += m21 * pt.y;
    result.w += m31 * pt.y;
    result.x += m02 * pt.z;
    result.y += m12 * pt.z;
    result.z += m22 * pt.z;
    result.w += m32 * pt.z;
    result.x += m03;
    result.y += m13;
    result.z += m23;
    result.w += m33;
    result.x /= result.w;
    result.y /= result.w;
    result.z /= result.w;
    
    return result;
}

Vector3f Matrix4x4::TransformVector(const Vector3f &v) const
{
    Vector3f result;
    
    result.x = m00 * v.x;
    result.y = m10 * v.x;
    result.z = m20 * v.x;
    result.x += m01 * v.y;
    result.y += m11 * v.y;
    result.z += m21 * v.y;
    result.x += m02 * v.z;
    result.y += m12 * v.z;
    result.z += m22 * v.z;
    
    return result;
}

Matrix4x4 Matrix4x4::GetIdentity()
{
    Matrix4x4 m;
    m.m00 = m.m11 = m.m22 = m.m33 = 1;
    return m;
}

float Matrix4x4::operator[](int index)
{
    return dataf[index];
}

Matrix4x4 Matrix4x4::FromTRS(Vector3f translate, Quaternion r, Vector3f scale)
{
    Matrix4x4 m0;
    r.ToMatrix(m0);
    
    m0.dataf[0] *= scale.x;
    m0.dataf[1] *= scale.x;
    m0.dataf[2] *= scale.x;
    m0.dataf[4] *= scale.y;
    m0.dataf[5] *= scale.y;
    m0.dataf[6] *= scale.y;
    m0.dataf[8] *= scale.z;
    m0.dataf[9] *= scale.z;
    m0.dataf[10] *= scale.z;
    m0.dataf[12] = translate.x;
    m0.dataf[13] = translate.y;
    m0.dataf[14] = translate.z;
    
    return m0;
}

Matrix4x4 Matrix4x4::FromTRS(Vector3f translate, Vector3f eulerAngle, EulerAngleOrder order, Vector3f scale)
{
    Matrix4x4 m0;
    EulerAngleToMatrix(eulerAngle, order, m0);
    
    m0.dataf[0] *= scale.x;
    m0.dataf[1] *= scale.x;
    m0.dataf[2] *= scale.x;
    m0.dataf[4] *= scale.y;
    m0.dataf[5] *= scale.y;
    m0.dataf[6] *= scale.y;
    m0.dataf[8] *= scale.z;
    m0.dataf[9] *= scale.z;
    m0.dataf[10] *= scale.z;
    m0.dataf[12] = translate.x;
    m0.dataf[13] = translate.y;
    m0.dataf[14] = translate.z;
    
    return m0;
}

#pragma endregion

#pragma region Matrix4x4

// cardan angle(Tait–Bryan angles), https://link.springer.com/content/pdf/bbm%3A978-1-4612-3502-6%2F1.pdf
void EulerAngleToMatrix(const Vector3f &eulerAngle, const EulerAngleOrder order, Matrix4x4 &matrix)
{
    float cx = cosf(eulerAngle.x * DEG2RAD), cy = cosf(eulerAngle.y * DEG2RAD), cz = cosf(eulerAngle.z * DEG2RAD);
    float sx = sinf(eulerAngle.x * DEG2RAD), sy = sinf(eulerAngle.y * DEG2RAD), sz = sinf(eulerAngle.z * DEG2RAD);
    
    switch (order) {
        case EulerAngleOrder::OrderXYZ:
            matrix.dataf[0] = cy * cz;
            matrix.dataf[4] = sx * sy * cz - cx * sz;
            matrix.dataf[8] = sx * sz + cx * sy * cz;
            
            matrix.dataf[1] = cy * sz;
            matrix.dataf[5] = cx * cz + sx * sy * sz;
            matrix.dataf[9] = cx * sy * sz - sx * cz;
            
            matrix.dataf[2]  = -sx;
            matrix.dataf[6]  = sx * cy;
            matrix.dataf[10] = cx * cy;
            break;
        case EulerAngleOrder::OrderXZY:
            matrix.dataf[0] = cy * cz;
            matrix.dataf[4] = sx * sy - cx * cy * sz;
            matrix.dataf[8] = sx * cy * sz + cx * sy;
            
            matrix.dataf[1] = sz;
            matrix.dataf[5] = cx * cz;
            matrix.dataf[9] = -sx * cz;
            
            matrix.dataf[2]  = -sy * cz;
            matrix.dataf[6]  = cx * sy * sz + sx * cy;
            matrix.dataf[10] = cx * cy - sx * sy * sz;
            break;
        case EulerAngleOrder::OrderYXZ:
            matrix.dataf[0] = cy * cz - sx * sy * sz;
            matrix.dataf[4] = -cx * sz;
            matrix.dataf[8] = sy * cz + sx * cy * sz;
            
            matrix.dataf[1] = cy * cz + sx * sy * cz;
            matrix.dataf[5] = cx * cz;
            matrix.dataf[9] = sy * sz - sx * cy * cx;
            
            matrix.dataf[2]  = -cx * sy;
            matrix.dataf[6]  = sx;
            matrix.dataf[10] = cx * cy;
            break;
        case EulerAngleOrder::OrderYZX:
            matrix.dataf[0] = cy * sz;
            matrix.dataf[4] = -sz;
            matrix.dataf[8] = sy * cz;
            
            matrix.dataf[1] = cx * cy * sz + sx * sy;
            matrix.dataf[5] = cx * cz;
            matrix.dataf[9] = cx * sy * sz - sx * cy;
            
            matrix.dataf[2]  = sx * cy * sz - cx * sy;
            matrix.dataf[6]  = sx * cz;
            matrix.dataf[10] = sx * sy * sz - cx * cy;
            break;
        case EulerAngleOrder::OrderZYX:
            matrix.dataf[0] = cy * cx;
            matrix.dataf[4] = -cy * sz;
            matrix.dataf[8] = sy;
            
            matrix.dataf[1] = cx * sy + sx * sy * cx;
            matrix.dataf[5] = cx * cz - sz * sy * sz;
            matrix.dataf[9] = -sx * cy;
            
            matrix.dataf[2]  = sx * sz - cx * sy * cz;
            matrix.dataf[6]  = sx * cz + cx * sy * sz;
            matrix.dataf[10] = cx * cy;
            break;
        case EulerAngleOrder::OrderZXY:
            matrix.dataf[0] = cy * cz + sx * sy * sz;
            matrix.dataf[1] = cx * sz;
            matrix.dataf[2] = sx * cy * sz - sy * cz;
            
            matrix.dataf[4] = -cy * sz;
            matrix.dataf[5] = cx * cz;
            matrix.dataf[6] = sy * sz + sx * cy * cz;
            
            matrix.dataf[8]  = cx * sy;
            matrix.dataf[9]  = -sx;
            matrix.dataf[10] = cx * cy;
            break;
    }
    
    matrix.dataf[3]  = 0;
    matrix.dataf[7]  = 0;
    matrix.dataf[11] = 0;
    
    matrix.dataf[12] = 0;
    matrix.dataf[13] = 0;
    matrix.dataf[14] = 0;
    matrix.dataf[15] = 1;
}

void EulerAngleToQuaternion(const Vector3f &eulerAngle, const EulerAngleOrder order, Quaternion &q)
{
}

#pragma endregion

#pragma region Ray

Ray::Ray()
{ }

Ray::Ray(const Ray &r) : origin(r.origin), direction(r.direction)
{ }

Ray::Ray(const Vector3f &o, const Vector3f &d) : origin(o), direction(d)
{ }

Ray &Ray::operator=(const Ray &r)
{
    this->origin    = r.origin;
    this->direction = r.direction;
    return *this;
}

Vector3f Ray::GetPosAt(float t) const
{
    return origin + direction * t;
}

Ray Ray::Inversed() const
{
    return Ray(origin, -direction);
}

#pragma endregion

#pragma region Bounds

Bounds::Bounds() : center(0, 0, 0), extents(0, 0, 0)
{ }

Bounds::Bounds(const Bounds &b) : center(b.center), extents(b.extents)
{ }

Bounds::Bounds(const Vector3f &c, const Vector3f &e) : center(c), extents(e)
{ }

Bounds &Bounds::operator=(const Bounds &v)
{
    this->center  = v.center;
    this->extents = v.extents;
    return *this;
}

bool Bounds::Intersect(const Ray &r) const
{
    // https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
    // r.dir is unit direction vector of ray
    Vector3f dir_frac = 1.0f / r.direction;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    Vector3f smp      = center - extents,
             bgp      = center + extents;
    float    t1       = (smp.x - r.origin.x) * dir_frac.x;
    float    t2       = (bgp.x - r.origin.x) * dir_frac.x;
    float    t3       = (smp.y - r.origin.y) * dir_frac.y;
    float    t4       = (bgp.y - r.origin.y) * dir_frac.y;
    float    t5       = (smp.z - r.origin.z) * dir_frac.z;
    float    t6       = (bgp.z - r.origin.z) * dir_frac.z;
    
    float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
    float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));
    
    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    // if tmin > tmax, ray doesn't intersect AABB
    return tmax >= 0 && tmin <= tmax;
}

bool Bounds::Intersect(const Ray &r, float &t) const
{
    // https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
    // r.dir is unit direction vector of ray
    Vector3f dir_frac = 1.0f / r.direction;
    // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
    // r.org is origin of ray
    Vector3f smp      = center - extents,
             bgp      = center + extents;
    float    t1       = (smp.x - r.origin.x) * dir_frac.x;
    float    t2       = (bgp.x - r.origin.x) * dir_frac.x;
    float    t3       = (smp.y - r.origin.y) * dir_frac.y;
    float    t4       = (bgp.y - r.origin.y) * dir_frac.y;
    float    t5       = (smp.z - r.origin.z) * dir_frac.z;
    float    t6       = (bgp.z - r.origin.z) * dir_frac.z;
    
    float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
    float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));
    
    // if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
    if (tmax < 0) {
        t = tmax;
        return false;
    }
    
    // if tmin > tmax, ray doesn't intersect AABB
    if (tmin > tmax) {
        t = tmax;
        return false;
    }
    
    t = tmin;
    return true;
}

void Bounds::Union(const Bounds &b)
{
    Vector3f
            min = Min(b.center - b.extents, center - extents),
            max = Max(b.center + b.extents, center + extents);
    
    center  = (max + min) / 2.f;
    extents = (max - min) / 2.f;
}

Bounds Bounds::Union(const Bounds &b0, const Bounds &b1)
{
    Vector3f
            min = Min(b0.center - b0.extents, b1.center - b1.extents),
            max = Max(b0.center + b0.extents, b1.center + b1.extents);
    
    return Bounds((min + max) / 2.f, (max - min) / 2.f);
}

void Bounds::Union(const Bounds &b0, const Bounds &b1, Bounds &b)
{
    Vector3f
            min = Min(b0.center - b0.extents, b1.center - b1.extents),
            max = Max(b0.center + b0.extents, b1.center + b1.extents);
    
    b = Bounds((max + min) / 2.f, (max - min) / 2.f);
}

#pragma endregion