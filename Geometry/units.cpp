#include "units.h"

#include "math_util.h"
#include "transforms.h"

#pragma region Vector2f

Vector2f::Vector2f() : x(0), y(0)
{ }

Vector2f::Vector2f(float x, float y) : x(x), y(y)
{ }

Vector2f::Vector2f(const Vector2f& v) : x(v.x), y(v.y)
{ }

float& Vector2f::operator[](uint32 i)
{
    return dataByfloat[i & 0x01];
}

const float& Vector2f::operator[](uint32 i) const
{
    return dataByfloat[i & 0x01];
}

const Vector2f& Vector2f::operator+() const
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

Vector2f& Vector2f::operator+=(const Vector2f& vec2)
{
    this->x += vec2.x;
    this->y += vec2.y;
    return *this;
}

Vector2f& Vector2f::operator-=(const Vector2f& vec2)
{
    this->x -= vec2.x;
    this->y -= vec2.y;
    return *this;
}

Vector2f& Vector2f::operator*=(const float t)
{
    this->x *= t;
    this->y *= t;
    return *this;
}

Vector2f& Vector2f::operator/=(const float t)
{
    this->x /= t;
    this->y /= t;
    return *this;
}

Vector2f& Vector2f::operator/=(const Vector2f& v)
{
    this->x /= v.x;
    this->y /= v.y;
    return *this;
}

Vector2f Vector2f::normalized() const
{
    return *this / magnitude();
}

Vector2f& Vector2f::normalize()
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

Vector2f operator/(const Vector2f& v, float t)
{
    return Vector2f(v.x / t, v.y / t);
}

Vector2f operator/(float t, const Vector2f& v)
{
    return Vector2f(t / v.x, t / v.y);
}

Vector2f operator/(const Vector2f& v1, const Vector2f& v2)
{
    return Vector2f(v1.x / v2.x, v1.y / v2.y);
}

bool operator==(const Vector2f& v1, const Vector2f& v2)
{
    return v1.x == v2.x && v1.y == v2.y;
}

bool operator!=(const Vector2f& v1, const Vector2f& v2)
{
    return v1.x != v2.x || v1.y != v2.y;
}

bool operator<(const Vector2f& v1, const Vector2f& v2)
{
    return v1.x < v2.x&& v1.y < v2.y;
}

bool operator<=(const Vector2f& v1, const Vector2f& v2)
{
    return v1.x <= v2.x && v1.y <= v2.y;
}

bool operator>(const Vector2f& v1, const Vector2f& v2)
{
    return v1.x > v2.x && v1.y > v2.y;
}

bool operator>=(const Vector2f& v1, const Vector2f& v2)
{
    return v1.x >= v2.x && v1.y >= v2.y;
}

float Dot(const Vector2f& v1, const Vector2f& v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

Vector2f Reflect(const Vector2f& v, const Vector2f& n)
{
    return v - 2 * Dot(v, n) * n;
}

const Vector2f Clamp(const Vector2f& val, const Vector2f& min, const Vector2f& max)
{
    return Vector2f(Clamp(val.x, min.x, max.x), Clamp(val.y, min.y, max.y));
}

const Vector2f Clamp(const Vector2f& val, float min, const Vector2f& max)
{
    return Vector2f(Clamp(val.x, min, max.x), Clamp(val.y, min, max.y));
}

const Vector2f Clamp(const Vector2f& val, const Vector2f& min, float max)
{
    return Vector2f(Clamp(val.x, min.x, max), Clamp(val.y, min.y, max));
}

const Vector2f Clamp(const Vector2f& val, float min, float max)
{
    return Vector2f(Clamp(val.x, min, max), Clamp(val.y, min, max));
}

Vector2f Min(const Vector2f& p1, const Vector2f& p2)
{
    return Vector2f(fmin(p1.x, p2.x), fmin(p1.y, p2.y));
}

Vector2f Max(const Vector2f& p1, const Vector2f& p2)
{
    return Vector2f(fmax(p1.x, p2.x), fmax(p1.y, p2.y));
}

Vector2f Abs(const Vector2f& v)
{
    return Vector2f(std::abs(v.x), std::abs(v.y));
}

Vector2f Sqrtv(const Vector2f& v)
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

Vector3f::Vector3f(const Vector2f& v, const float& o) : x(v.x), y(v.y), z(o)
{ }

Vector3f::Vector3f(const float& o, const Vector2f& v) : x(o), y(v.x), z(v.y)
{ }

Vector3f::Vector3f(const Vector3f& v) : x(v.x), y(v.y), z(v.z)
{ }

Vector3f::operator Vector2f() const
{
    return Vector2f(this->x, this->y);
}

Vector3f::operator Vector4f() const
{
    return Vector4f(this->x, this->y, this->z, 0);
}

float& Vector3f::operator[](uint32 i)
{
    return dataByfloat[i % 3];
}

const float& Vector3f::operator[](uint32 i) const
{
    return dataByfloat[i % 3];
}

const Vector3f& Vector3f::operator+() const
{
    return *this;
}

Vector3f Vector3f::operator-() const
{
    return Vector3f(-x, -y, -z);
}

Vector3f& Vector3f::operator+=(const Vector3f& vec3)
{
    this->x += vec3.x;
    this->y += vec3.y;
    this->z += vec3.z;
    return *this;
}

Vector3f& Vector3f::operator-=(const Vector3f& vec3)
{
    this->x -= vec3.x;
    this->y -= vec3.y;
    this->z -= vec3.z;
    return *this;
}

Vector3f& Vector3f::operator*=(const float t)
{
    this->x *= t;
    this->y *= t;
    this->z *= t;
    return *this;
}

Vector3f& Vector3f::operator/=(const float t)
{
    this->x /= t;
    this->y /= t;
    this->z /= t;
    return *this;
}

Vector3f& Vector3f::operator/=(const Vector3f& v)
{
    this->x /= v.x;
    this->y /= v.y;
    this->z /= v.z;
    return *this;
}

Vector3f& Vector3f::operator=(const Vector3f& v)
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

Vector3f& Vector3f::normalize()
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

bool operator==(const Vector3f& v1, const Vector3f& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

bool operator!=(const Vector3f& v1, const Vector3f& v2)
{
    return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z;
}

bool operator<(const Vector3f& v1, const Vector3f& v2)
{
    return v1.x < v2.x&& v1.y < v2.y&& v1.z < v2.z;
}

bool operator<=(const Vector3f& v1, const Vector3f& v2)
{
    return v1.x <= v2.x && v1.y <= v2.y && v1.z <= v2.z;
}

bool operator>(const Vector3f& v1, const Vector3f& v2)
{
    return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z;
}

bool operator>=(const Vector3f& v1, const Vector3f& v2)
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

Vector3f operator/(const Vector3f& v, float t)
{
    return Vector3f(v.x / t, v.y / t, v.z / t);
}

Vector3f operator/(float t, const Vector3f& v)
{
    return Vector3f(t / v.x, t / v.y, t / v.z);
}

Vector3f operator/(const Vector3f& v1, const Vector3f& v2)
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

Vector3f Reflect(const Vector3f& v, const Vector3f& n)
{
    return v - 2 * Dot(v, n) * n;
}

bool Refract(const Vector3f& v, const Vector3f& n, float ni_over_nt, Vector3f& refracted)
{
    Vector3f uv = v.normalized();
    float    dt = Dot(uv, n);
    float    discriminant = 1.f - ni_over_nt * ni_over_nt * (1 - dt * dt);
    if (discriminant > 0) {
        refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
        return true;
    }
    else
        return false;
}

const Vector3f Clamp(const Vector3f& val, const Vector3f& min, const Vector3f& max)
{
    return Vector3f(Clamp(val.x, min.x, max.x), Clamp(val.y, min.y, max.y), Clamp(val.z, min.z, max.z));
}

const Vector3f Clamp(const Vector3f& val, float min, const Vector3f& max)
{
    return Vector3f(Clamp(val.x, min, max.x), Clamp(val.y, min, max.y), Clamp(val.z, min, max.z));
}

const Vector3f Clamp(const Vector3f& val, const Vector3f& min, float max)
{
    return Vector3f(Clamp(val.x, min.x, max), Clamp(val.y, min.y, max), Clamp(val.z, min.z, max));
}

const Vector3f Clamp(const Vector3f& val, float min, float max)
{
    return Vector3f(Clamp(val.x, min, max), Clamp(val.y, min, max), Clamp(val.z, min, max));
}

Vector3f Min(const Vector3f& p1, const Vector3f& p2)
{
    return Vector3f(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
}

Vector3f Max(const Vector3f& p1, const Vector3f& p2)
{
    return Vector3f(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
}

Vector3f Abs(const Vector3f& v)
{
    return Vector3f(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

Vector3f Sqrtv(const Vector3f& v)
{
    return Vector3f(std::sqrt(v.x), std::sqrt(v.y), std::sqrt(v.z));
}

#pragma endregion

#pragma region Vector4f

Vector4f::Vector4f() : x(0), y(0), z(0), w(0)
{ }

Vector4f::Vector4f(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{ }

Vector4f::Vector4f(const Vector2f& v0, const Vector2f& v1) : x(v0.x), y(v0.y), z(v1.x), w(v1.y)
{ }

Vector4f::Vector4f(const Vector2f& v, const float& o0, const float& o1) : x(v.x), y(v.y), z(o0), w(o1)
{ }

Vector4f::Vector4f(const float& o0, const Vector2f& v, const float& o1) : x(o0), y(v.x), z(v.y), w(o1)
{ }

Vector4f::Vector4f(const float& o0, const float& o1, const Vector2f& v) : x(o0), y(o1), z(v.x), w(v.y)
{ }

Vector4f::Vector4f(const Vector3f& v, const float& o) : x(v.x), y(v.y), z(v.z), w(o)
{ }

Vector4f::Vector4f(const float& o, const Vector3f& v) : x(o), y(v.x), z(v.y), w(v.z)
{ }

Vector4f::Vector4f(const Vector4f& v) : x(v.x), y(v.y), z(v.z), w(v.w)
{ }

Vector4f::operator Vector2f() const
{
    return Vector2f(this->x, this->y);
}

Vector4f::operator Vector3f() const
{
    return Vector3f(this->x, this->y, this->z);
}

float& Vector4f::operator[](uint32 i)
{
    return dataByfloat[i & 0x03];
}

const float& Vector4f::operator[](uint32 i) const
{
    return dataByfloat[i & 0x03];
}

const Vector4f& Vector4f::operator+() const
{
    return *this;
}

Vector4f Vector4f::operator-() const
{
    return Vector4f(-x, -y, -z, -w);
}

Vector4f& Vector4f::operator+=(const Vector4f& vec3)
{
    this->x += vec3.x;
    this->y += vec3.y;
    this->z += vec3.z;
    this->w += vec3.w;
    return *this;
}

Vector4f& Vector4f::operator-=(const Vector4f& vec3)
{
    this->x -= vec3.x;
    this->y -= vec3.y;
    this->z -= vec3.z;
    this->w -= vec3.w;
    return *this;
}

Vector4f& Vector4f::operator*=(const float t)
{
    this->x *= t;
    this->y *= t;
    this->z *= t;
    this->w *= t;
    return *this;
}

Vector4f& Vector4f::operator/=(const float t)
{
    this->x /= t;
    this->y /= t;
    this->z /= t;
    this->w /= t;
    return *this;
}

Vector4f& Vector4f::operator/=(const Vector4f& v)
{
    this->x /= v.x;
    this->y /= v.y;
    this->z /= v.z;
    this->w /= v.w;
    return *this;
}

Vector4f& Vector4f::operator*=(const Vector4f& v)
{
    this->x *= v.x;
    this->y *= v.y;
    this->z *= v.z;
    this->w *= v.w;
    return *this;
}

float Vector4f::magnitude() const
{
    return sqrt(x * x + y * y + z * z + w * w);
}

float Vector4f::sqrMagnitude() const
{
    return x * x + y * y + z * z + w * w;
}

Vector4f Vector4f::normalized() const
{
    return *this / magnitude();
}

Vector4f& Vector4f::normalize()
{
    return *this /= magnitude();
}

Vector4f Vector4f::Zero()
{
    return Vector4f(0, 0, 0, 0);
}

Vector4f Vector4f::One()
{
    return Vector4f(1, 1, 1, 1);
}

Vector3f Vector4f::GetVec3()
{
    return Vector3f(x, y, z);
}

int Vector4f::MaxDimension()
{
    auto xyPtr = (x > y) ? &x : &y;
    auto zwPtr = (z > w) ? &z : &w;
    auto lastPtr = (*xyPtr > *zwPtr) ? xyPtr : zwPtr;
    auto diff = lastPtr - reinterpret_cast<const float*>(this);

    return (int)(diff / sizeof(float));
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

bool operator==(const Vector4f& v1, const Vector4f& v2)
{
    return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

bool operator!=(const Vector4f& v1, const Vector4f& v2)
{
    return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z || v1.w != v2.w;
}

bool operator<(const Vector4f& v1, const Vector4f& v2)
{
    return v1.x < v2.x&& v1.y < v2.y&& v1.z < v2.z&& v1.w < v2.w;
}

bool operator<=(const Vector4f& v1, const Vector4f& v2)
{
    return v1.x <= v2.x && v1.y <= v2.y && v1.z <= v2.z && v1.w <= v2.w;
}

bool operator>(const Vector4f& v1, const Vector4f& v2)
{
    return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z && v1.w > v2.w;
}

bool operator>=(const Vector4f& v1, const Vector4f& v2)
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

Vector4f operator/(const Vector4f& v1, const Vector4f& v2)
{
    return Vector4f(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

float Dot(Vector4f v1, Vector4f v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

Vector4f Reflect(const Vector4f& v, const Vector4f& n)
{
    return v - 2 * Dot(v, n) * n;
}

Vector4f Clamp(const Vector4f& val, const Vector4f& min, const Vector4f& max)
{
    return Vector4f(
        Clamp(val.x, min.x, max.x), Clamp(val.y, min.y, max.y),
        Clamp(val.z, min.z, max.z), Clamp(val.w, min.w, max.w)
    );
}

Vector4f Clamp(const Vector4f& val, float min, const Vector4f& max)
{
    return Vector4f(
        Clamp(val.x, min, max.x), Clamp(val.y, min, max.y),
        Clamp(val.z, min, max.z), Clamp(val.w, min, max.w)
    );
}

Vector4f Clamp(const Vector4f& val, const Vector4f& min, float max)
{
    return Vector4f(
        Clamp(val.x, min.x, max), Clamp(val.y, min.y, max),
        Clamp(val.z, min.z, max), Clamp(val.w, min.w, max)
    );
}

Vector4f Clamp(const Vector4f& val, float min, float max)
{
    return Vector4f(
        Clamp(val.x, min, max), Clamp(val.y, min, max),
        Clamp(val.z, min, max), Clamp(val.w, min, max)
    );
}

Vector4f Min(const Vector4f& p1, const Vector4f& p2)
{
    return Vector4f(
        fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z), fmin(p1.w, p2.w)
    );
}

Vector4f Max(const Vector4f& p1, const Vector4f& p2)
{
    return Vector4f(
        fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.x, p2.x), fmax(p1.y, p2.y)
    );
}

Vector4f Abs(const Vector4f& v)
{
    return Vector4f(fabs(v.x), fabs(v.y), fabs(v.z), fabs(v.w));
}

Vector4f Sqrtv(const Vector4f& v)
{
    return Vector4f(std::sqrt(v.x), std::sqrt(v.y), std::sqrt(v.z), std::sqrt(v.w));
}

#pragma endregion

#pragma region Quaternion

Quaternion::Quaternion()
{ }

Quaternion::Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w)
{ }

Quaternion::Quaternion(const Quaternion& q) : x(q.x), y(q.y), z(q.z), w(q.w)
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
{
    return Quaternion(0, 0, 0, 1);
}

Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
{
    return Quaternion(
        q2.w * q1.x + q2.x * q1.w + q2.y * q1.z - q2.z * q1.y,
        q2.w * q1.y + q2.y * q1.w + q2.z * q1.x - q2.x * q1.z,
        q2.w * q1.z + q2.z * q1.w + q2.x * q1.y - q2.y * q1.x,
        q2.w * q1.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z);
}

Vector3f operator*(const Quaternion& q1, const Vector3f& v1)
{
    Vector3f v = v1;
    Rotate(q1, v);
    return v;
}

Vector3f& operator*(const Quaternion& q1, Vector3f& v1)
{
    Rotate(q1, v1);
    return v1;
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

Matrix4x4::Matrix4x4(const Matrix4x4& mat) :
    m00(mat.m00), m10(mat.m10), m20(mat.m20), m30(mat.m30),
    m01(mat.m01), m11(mat.m11), m21(mat.m21), m31(mat.m31),
    m02(mat.m02), m12(mat.m12), m22(mat.m22), m32(mat.m32),
    m03(mat.m03), m13(mat.m13), m23(mat.m23), m33(mat.m33)
{ }

Matrix4x4::Matrix4x4(const Vector4f& c0, const Vector4f& c1, const Vector4f& c2, const Vector4f& c3)
{
    this->c0 = c0;
    this->c1 = c1;
    this->c2 = c2;
    this->c3 = c3;
}

Matrix4x4 Matrix4x4::Transpose() const
{
    return Matrix4x4(this->GetRow(0), this->GetRow(1), this->GetRow(2), this->GetRow(3));
}

void Matrix4x4::Transpose()
{
    auto temp = (float)0;

    temp = m10;
    m10 = m01;
    m01 = temp;

    temp = m20;
    m20 = m02;
    m02 = temp;

    temp = m21;
    m21 = m12;
    m12 = temp;

    temp = m30;
    m30 = m03;
    m03 = temp;

    temp = m31;
    m31 = m13;
    m13 = temp;

    temp = m32;
    m32 = m23;
    m23 = temp;
}

// https://stackoverflow.com/questions/2624422/efficient-4x4-matrix-inverse-affine-transform
float Matrix4x4::Determinant() const
{
    auto s0 = this->columns[0][0] * this->columns[1][1] - this->columns[1][0] * this->columns[0][1];
    auto s1 = this->columns[0][0] * this->columns[1][2] - this->columns[1][0] * this->columns[0][2];
    auto s2 = this->columns[0][0] * this->columns[1][3] - this->columns[1][0] * this->columns[0][3];
    auto s3 = this->columns[0][1] * this->columns[1][2] - this->columns[1][1] * this->columns[0][2];
    auto s4 = this->columns[0][1] * this->columns[1][3] - this->columns[1][1] * this->columns[0][3];
    auto s5 = this->columns[0][2] * this->columns[1][3] - this->columns[1][2] * this->columns[0][3];

    auto c5 = this->columns[2][2] * this->columns[3][3] - this->columns[3][2] * this->columns[2][3];
    auto c4 = this->columns[2][1] * this->columns[3][3] - this->columns[3][1] * this->columns[2][3];
    auto c3 = this->columns[2][1] * this->columns[3][2] - this->columns[3][1] * this->columns[2][2];
    auto c2 = this->columns[2][0] * this->columns[3][3] - this->columns[3][0] * this->columns[2][3];
    auto c1 = this->columns[2][0] * this->columns[3][2] - this->columns[3][0] * this->columns[2][2];
    auto c0 = this->columns[2][0] * this->columns[3][1] - this->columns[3][0] * this->columns[2][1];

    return (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);
}

bool Matrix4x4::Inverse(Matrix4x4& invMat) const
{
    auto s0 = this->columns[0][0] * this->columns[1][1] - this->columns[1][0] * this->columns[0][1];
    auto s1 = this->columns[0][0] * this->columns[1][2] - this->columns[1][0] * this->columns[0][2];
    auto s2 = this->columns[0][0] * this->columns[1][3] - this->columns[1][0] * this->columns[0][3];
    auto s3 = this->columns[0][1] * this->columns[1][2] - this->columns[1][1] * this->columns[0][2];
    auto s4 = this->columns[0][1] * this->columns[1][3] - this->columns[1][1] * this->columns[0][3];
    auto s5 = this->columns[0][2] * this->columns[1][3] - this->columns[1][2] * this->columns[0][3];

    auto c5 = this->columns[2][2] * this->columns[3][3] - this->columns[3][2] * this->columns[2][3];
    auto c4 = this->columns[2][1] * this->columns[3][3] - this->columns[3][1] * this->columns[2][3];
    auto c3 = this->columns[2][1] * this->columns[3][2] - this->columns[3][1] * this->columns[2][2];
    auto c2 = this->columns[2][0] * this->columns[3][3] - this->columns[3][0] * this->columns[2][3];
    auto c1 = this->columns[2][0] * this->columns[3][2] - this->columns[3][0] * this->columns[2][2];
    auto c0 = this->columns[2][0] * this->columns[3][1] - this->columns[3][0] * this->columns[2][1];

    auto det = (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);
    if (det == 0) {
        return false;
    }
    auto invdet = 1.0 / det;
    
    invMat.columns[0][0] = (this->columns[1][1] * c5 - this->columns[1][2] * c4 + this->columns[1][3] * c3) * invdet;
    invMat.columns[0][1] = (-this->columns[0][1] * c5 + this->columns[0][2] * c4 - this->columns[0][3] * c3) * invdet;
    invMat.columns[0][2] = (this->columns[3][1] * s5 - this->columns[3][2] * s4 + this->columns[3][3] * s3) * invdet;
    invMat.columns[0][3] = (-this->columns[2][1] * s5 + this->columns[2][2] * s4 - this->columns[2][3] * s3) * invdet;

    invMat.columns[1][0] = (-this->columns[1][0] * c5 + this->columns[1][2] * c2 - this->columns[1][3] * c1) * invdet;
    invMat.columns[1][1] = (this->columns[0][0] * c5 - this->columns[0][2] * c2 + this->columns[0][3] * c1) * invdet;
    invMat.columns[1][2] = (-this->columns[3][0] * s5 + this->columns[3][2] * s2 - this->columns[3][3] * s1) * invdet;
    invMat.columns[1][3] = (this->columns[2][0] * s5 - this->columns[2][2] * s2 + this->columns[2][3] * s1) * invdet;

    invMat.columns[2][0] = (this->columns[1][0] * c4 - this->columns[1][1] * c2 + this->columns[1][3] * c0) * invdet;
    invMat.columns[2][1] = (-this->columns[0][0] * c4 + this->columns[0][1] * c2 - this->columns[0][3] * c0) * invdet;
    invMat.columns[2][2] = (this->columns[3][0] * s4 - this->columns[3][1] * s2 + this->columns[3][3] * s0) * invdet;
    invMat.columns[2][3] = (-this->columns[2][0] * s4 + this->columns[2][1] * s2 - this->columns[2][3] * s0) * invdet;

    invMat.columns[3][0] = (-this->columns[1][0] * c3 + this->columns[1][1] * c1 - this->columns[1][2] * c0) * invdet;
    invMat.columns[3][1] = (this->columns[0][0] * c3 - this->columns[0][1] * c1 + this->columns[0][2] * c0) * invdet;
    invMat.columns[3][2] = (-this->columns[3][0] * s3 + this->columns[3][1] * s1 - this->columns[3][2] * s0) * invdet;
    invMat.columns[3][3] = (this->columns[2][0] * s3 - this->columns[2][1] * s1 + this->columns[2][2] * s0) * invdet;

    return true;
}

bool Matrix4x4::Inverse()
{
    auto s0 = this->columns[0][0] * this->columns[1][1] - this->columns[1][0] * this->columns[0][1];
    auto s1 = this->columns[0][0] * this->columns[1][2] - this->columns[1][0] * this->columns[0][2];
    auto s2 = this->columns[0][0] * this->columns[1][3] - this->columns[1][0] * this->columns[0][3];
    auto s3 = this->columns[0][1] * this->columns[1][2] - this->columns[1][1] * this->columns[0][2];
    auto s4 = this->columns[0][1] * this->columns[1][3] - this->columns[1][1] * this->columns[0][3];
    auto s5 = this->columns[0][2] * this->columns[1][3] - this->columns[1][2] * this->columns[0][3];

    auto c5 = this->columns[2][2] * this->columns[3][3] - this->columns[3][2] * this->columns[2][3];
    auto c4 = this->columns[2][1] * this->columns[3][3] - this->columns[3][1] * this->columns[2][3];
    auto c3 = this->columns[2][1] * this->columns[3][2] - this->columns[3][1] * this->columns[2][2];
    auto c2 = this->columns[2][0] * this->columns[3][3] - this->columns[3][0] * this->columns[2][3];
    auto c1 = this->columns[2][0] * this->columns[3][2] - this->columns[3][0] * this->columns[2][2];
    auto c0 = this->columns[2][0] * this->columns[3][1] - this->columns[3][0] * this->columns[2][1];

    auto det = (s0 * c5 - s1 * c4 + s2 * c3 + s3 * c2 - s4 * c1 + s5 * c0);
    if (det == 0) {
        return false;
    }
    auto invdet = 1.0 / det;

    auto dst = Matrix4x4();

    dst.columns[0][0] = (this->columns[1][1] * c5 - this->columns[1][2] * c4 + this->columns[1][3] * c3) * invdet;
    dst.columns[0][1] = (-this->columns[0][1] * c5 + this->columns[0][2] * c4 - this->columns[0][3] * c3) * invdet;
    dst.columns[0][2] = (this->columns[3][1] * s5 - this->columns[3][2] * s4 + this->columns[3][3] * s3) * invdet;
    dst.columns[0][3] = (-this->columns[2][1] * s5 + this->columns[2][2] * s4 - this->columns[2][3] * s3) * invdet;

    dst.columns[1][0] = (-this->columns[1][0] * c5 + this->columns[1][2] * c2 - this->columns[1][3] * c1) * invdet;
    dst.columns[1][1] = (this->columns[0][0] * c5 - this->columns[0][2] * c2 + this->columns[0][3] * c1) * invdet;
    dst.columns[1][2] = (-this->columns[3][0] * s5 + this->columns[3][2] * s2 - this->columns[3][3] * s1) * invdet;
    dst.columns[1][3] = (this->columns[2][0] * s5 - this->columns[2][2] * s2 + this->columns[2][3] * s1) * invdet;

    dst.columns[2][0] = (this->columns[1][0] * c4 - this->columns[1][1] * c2 + this->columns[1][3] * c0) * invdet;
    dst.columns[2][1] = (-this->columns[0][0] * c4 + this->columns[0][1] * c2 - this->columns[0][3] * c0) * invdet;
    dst.columns[2][2] = (this->columns[3][0] * s4 - this->columns[3][1] * s2 + this->columns[3][3] * s0) * invdet;
    dst.columns[2][3] = (-this->columns[2][0] * s4 + this->columns[2][1] * s2 - this->columns[2][3] * s0) * invdet;

    dst.columns[3][0] = (-this->columns[1][0] * c3 + this->columns[1][1] * c1 - this->columns[1][2] * c0) * invdet;
    dst.columns[3][1] = (this->columns[0][0] * c3 - this->columns[0][1] * c1 + this->columns[0][2] * c0) * invdet;
    dst.columns[3][2] = (-this->columns[3][0] * s3 + this->columns[3][1] * s1 - this->columns[3][2] * s0) * invdet;
    dst.columns[3][3] = (this->columns[2][0] * s3 - this->columns[2][1] * s1 + this->columns[2][2] * s0) * invdet;

    *this = dst;
    return true;
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

bool Matrix4x4::SetColumn(int index, const Vector4f& c)
{
    if (index < 0 || 4 <= index) {
        return false;
    }

    columns[index] = c;
    return true;
}

bool Matrix4x4::SetRow(int index, const Vector4f& r)
{
    if (index < 0 || 4 <= index) {
        return false;
    }

    dataf[0 * 4 + index] = r[0];
    dataf[1 * 4 + index] = r[1];
    dataf[2 * 4 + index] = r[2]; 
    dataf[3 * 4 + index] = r[3];

    return true;
}

Vector3f Matrix4x4::TransformPoint(const Vector3f& pt) const
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

Vector3f Matrix4x4::TransformVector(const Vector3f& v) const
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

Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2)
{
    Matrix4x4 m;

    /*  m1                 m2
        m00 m01 m02 m03    m00 m01 m02 m03
        m10 m11 m12 m13    m10 m11 m12 m13
        m20 m21 m22 m23    m20 m21 m22 m23
        m30 m31 m32 m33    m30 m31 m32 m33
    */

    m.m00 =
        m1.m00 * m2.m00 +
        m1.m01 * m2.m10 +
        m1.m02 * m2.m20 +
        m1.m03 * m2.m30;

    m.m10 =
        m1.m10 * m2.m00 +
        m1.m11 * m2.m10 +
        m1.m12 * m2.m20 +
        m1.m13 * m2.m30;

    m.m20 =
        m1.m20 * m2.m00 +
        m1.m21 * m2.m10 +
        m1.m22 * m2.m20 +
        m1.m23 * m2.m30;

    m.m30 =
        m1.m30 * m2.m00 +
        m1.m31 * m2.m10 +
        m1.m32 * m2.m20 +
        m1.m33 * m2.m30;

    m.m01 =
        m1.m00 * m2.m01 +
        m1.m01 * m2.m11 +
        m1.m02 * m2.m21 +
        m1.m03 * m2.m31;

    m.m11 =
        m1.m10 * m2.m01 +
        m1.m11 * m2.m11 +
        m1.m12 * m2.m21 +
        m1.m13 * m2.m31;

    m.m21 =
        m1.m20 * m2.m01 +
        m1.m21 * m2.m11 +
        m1.m22 * m2.m21 +
        m1.m23 * m2.m31;

    m.m31 =
        m1.m30 * m2.m01 +
        m1.m31 * m2.m11 +
        m1.m32 * m2.m21 +
        m1.m33 * m2.m31;

    m.m02 =
        m1.m00 * m2.m02 +
        m1.m01 * m2.m12 +
        m1.m02 * m2.m22 +
        m1.m03 * m2.m32;

    m.m12 =
        m1.m10 * m2.m02 +
        m1.m11 * m2.m12 +
        m1.m12 * m2.m22 +
        m1.m13 * m2.m32;

    m.m22 =
        m1.m20 * m2.m02 +
        m1.m21 * m2.m12 +
        m1.m22 * m2.m22 +
        m1.m23 * m2.m32;

    m.m32 =
        m1.m30 * m2.m02 +
        m1.m31 * m2.m12 +
        m1.m32 * m2.m22 +
        m1.m33 * m2.m32;

    m.m03 =
        m1.m00 * m2.m03 +
        m1.m01 * m2.m13 +
        m1.m02 * m2.m23 +
        m1.m03 * m2.m33;

    m.m13 =
        m1.m10 * m2.m03 +
        m1.m11 * m2.m13 +
        m1.m12 * m2.m23 +
        m1.m13 * m2.m33;

    m.m23 =
        m1.m20 * m2.m03 +
        m1.m21 * m2.m13 +
        m1.m22 * m2.m23 +
        m1.m23 * m2.m33;

    m.m33 =
        m1.m30 * m2.m03 +
        m1.m31 * m2.m13 +
        m1.m32 * m2.m23 +
        m1.m33 * m2.m33;

    return m;
}

#pragma endregion
