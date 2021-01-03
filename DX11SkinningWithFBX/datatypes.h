#include "defined.h"
#include <iostream>

#pragma once

#define SQRT_OF_ONE_THIRD 0.5773502691896257645091487805019574556476f
#define EPSILON           0.001f
#define Pi2				(6.28318530717958647692f)
#define Pi				(3.14159265358979323846f)
#define InvPi			(0.31830988618379067154f)
#define Inv2Pi			(0.15915494309189533577f)
#define Inv4Pi			(0.07957747154594766788f)
#define PiOver2			(1.57079632679489661923f)
#define PiOver4			(0.78539816339744830961f)
#define Sqrt2			(1.41421356237309504880f)
#define DEG2RAD			(Pi / 180.0f)
#define RAD2DEG			(180.0f / Pi)

inline float Lerp(float start, float end, float norm)
{
	return start * (1.f - norm) + end * norm;
}
inline double Lerp(double start, double end, double norm)
{
	return start * (1. - norm) + end * norm;
}
inline long double Lerp(long double start, long double end, long double norm)
{
	return start * (1.L - norm) + end * norm;
}

template <typename Type>
inline Type Abs(Type val)
{
	if (val < 0)
		return -val;
	else
		return val;
}

template <typename Type>
inline float Abs(float val)
{
	return fabs(val);
}

template <typename Type>
inline double Abs(double val)
{
	return fabs(val);
}

template <typename Type>
inline long double Abs(long double val)
{
	return fabs(val);
}

template <typename Type>
inline Type Clamp(Type val, Type min, Type max)
{
	if (val < min)
		return min;
	else if (val > max)
		return max;
	else
		return val;
}

template <typename Type>
inline float Clamp(float val, float min, float max)
{
	return fmax(min, fmin(max, val));
}
template <typename Type>
inline double Clamp(double val, double min, double max)
{
	return fmax(min, fmin(max, val));
}
template <typename Type>
inline long double Clamp(long double val, long double min, long double max)
{
	return fmax(min, fmin(max, val));
}

template <typename Type>
struct Vector2;
template <typename Type>
struct Vector3;
template <typename Type>
struct Vector4;

template <typename Type>
struct Vector2
{
	union
	{
		struct
		{
			Type x;
			Type y;
		};
		Type dataByType[2];
		byte data[2 * sizeof(Type)];
	};

	Vector2() : x(0), y(0) { }
	Vector2(Type x, Type y) : x(x), y(y) {}
	Vector2(const Vector2<Type>& v) : x(v.x), y(v.y) {}

	operator Vector3<Type>() const;
	operator Vector4<Type>() const;

	Type& operator[] (unsigned int i)
	{
		return dataByType[i & 0x01];
	}

	const Type& operator[] (unsigned int i) const
	{
		return dataByType[i & 0x01];
	}

	const Vector2<Type>& operator+() const { return *this; }
	Vector2<Type> operator-() const { return Vector2<Type>(-x, -y); }

	Vector2<Type>& operator+=(const Vector2<Type>& vec2)
	{
		this->x += vec2.x;
		this->y += vec2.y;
		return *this;
	}
	Vector2<Type>& operator-=(const Vector2<Type>& vec2)
	{
		this->x -= vec2.x;
		this->y -= vec2.y;
		return *this;
	}
	Vector2<Type>& operator*=(const Type t)
	{
		this->x *= t;
		this->y *= t;
		return *this;
	}
	Vector2<Type>& operator/=(const Type t)
	{
		this->x /= t;
		this->y /= t;
		return *this;
	}
	Vector2<Type>& operator/=(const Vector2<Type>& v)
	{
		this->x /= v.x;
		this->y /= v.y;
		return *this;
	}

	Type magnitude() const { return sqrt(x*x + y * y); }
	Type sqrMagnitude() const { return x * x + y * y; }

	Vector2<Type> normalized() const { return *this / magnitude(); }
	Vector2<Type>& normalize() { return *this /= magnitude(); }

	static Vector2<Type> Zero() { return Vector2<Type>(0, 0); }
	static Vector2<Type> One() { return Vector2<Type>(1, 1); }
};

template <typename Type>
Type Dot(Vector2<Type> v1, Vector2<Type> v2)
{
	return v1.x * v2.x + v1.y * v2.y;
}

template <typename Type>
Vector2<Type> Reflect(const Vector2<Type>& v, const Vector2<Type>& n)
{
	return v - 2 * Dot<Type>(v, n) * n;
}

template <typename Type>
bool operator==(const Vector2<Type>& v1, const Vector2<Type>& v2)
{
	return v1.x == v2.x && v1.y == v2.y;
}

template <typename Type>
bool operator!=(const Vector2<Type>& v1, const Vector2<Type>& v2)
{
	return v1.x != v2.x || v1.y != v2.y;
}

template <typename Type>
bool operator<(const Vector2<Type>& v1, const Vector2<Type>& v2)
{
	return v1.x < v2.x && v1.y < v2.y;
}

template <typename Type>
bool operator<=(const Vector2<Type>& v1, const Vector2<Type>& v2)
{
	return v1.x <= v2.x && v1.y <= v2.y;
}

template <typename Type>
bool operator>(const Vector2<Type>& v1, const Vector2<Type>& v2)
{
	return v1.x > v2.x && v1.y > v2.y;
}

template <typename Type>
bool operator>=(const Vector2<Type>& v1, const Vector2<Type>& v2)
{
	return v1.x >= v2.x && v1.y >= v2.y;
}


template <typename Type>
Vector2<Type> operator+(Vector2<Type> v1, Vector2<Type> v2)
{
	return Vector2<Type>(v1.x + v2.x, v1.y + v2.y);
}

template <typename Type>
Vector2<Type> operator-(Vector2<Type> v1, Vector2<Type> v2)
{
	return Vector2<Type>(v1.x - v2.x, v1.y - v2.y);
}

template <typename Type>
Vector2<Type> operator*(Vector2<Type> v, Type t)
{
	return Vector2<Type>(v.x * t, v.y * t);
}

template <typename Type>
Vector2<Type> operator*(Vector2<Type> v1, Vector2<Type> v2)
{
	return Vector2<Type>(v1.x * v2.x, v1.y * v2.y);
}

template <typename Type>
Vector2<Type> operator*(Type t, Vector2<Type> v)
{
	return Vector2<Type>(v.x * t, v.y * t);
}

template <typename Type>
Vector2<Type> operator/(const Vector2<Type>& v, Type t)
{
	return Vector2<Type>(v.x / t, v.y / t);
}

template <typename Type>
Vector2<Type> operator/(Type t, const Vector2<Type>& v)
{
	return Vector2<Type>(t / v.x, t / v.y);
}

template <typename Type>
Vector2<Type> operator/(const Vector2<Type>& v1, const Vector2<Type>& v2)
{
	return Vector2<Type>(v1.x / v2.x, v1.y / v2.y);
}

template <typename Type>
std::istream& operator>>(std::istream &i, Vector2<Type>& v)
{
	i >> v.x >> v.y;
	return i;
}

template <typename Type>
std::ostream& operator<<(std::ostream &o, Vector2<Type>& v)
{
	o << v.x << ' ' << v.y;
	return o;
}

typedef Vector2<float>				Vector2f;
typedef Vector2<int>				Vector2i;
typedef Vector2<unsigned int>		Vector2u;

inline Vector2i Round(const Vector2f& vf)
{
	return Vector2i(int(vf.x + 0.5), int(vf.y + 0.5));
}

template <typename Type>
struct Vector3
{
	union
	{
		struct
		{
			Type x;
			Type y;
			Type z;
		};
		Type dataByType[3];
		byte data[3 * sizeof(Type)];
	};

	Vector3() : x(0), y(0), z(0) { }
	Vector3(Type val) : x(val), y(val), z(val) {}
	Vector3(Type x, Type y, Type z) : x(x), y(y), z(z) {}
	Vector3(const Vector2<Type>& v, const Type& o) : x(v.x), y(v.y), z(o) {}
	Vector3(const Type& o, const Vector2<Type>& v) : x(o), y(v.x), z(v.y) {}
	Vector3(const Vector3<Type>& v) : x(v.x), y(v.y), z(v.z) {}

	operator Vector2<Type>() const;
	operator Vector4<Type>() const;

	Type& operator[] (unsigned int i)
	{
		return dataByType[i % 3];
	}

	const Type& operator[] (unsigned int i) const
	{
		return dataByType[i % 3];
	}

	const Vector3<Type>& operator+() const { return *this; }
	Vector3<Type> operator-() const { return Vector3<Type>(-x, -y, -z); }

	Vector3<Type>& operator+=(const Vector3<Type>& vec3)
	{
		this->x += vec3.x;
		this->y += vec3.y;
		this->z += vec3.z;
		return *this;
	}
	Vector3<Type>& operator-=(const Vector3<Type>& vec3)
	{
		this->x -= vec3.x;
		this->y -= vec3.y;
		this->z -= vec3.z;
		return *this;
	}
	Vector3<Type>& operator*=(const Type t)
	{
		this->x *= t;
		this->y *= t;
		this->z *= t;
		return *this;
	}
	Vector3<Type>& operator/=(const Type t)
	{
		this->x /= t;
		this->y /= t;
		this->z /= t;
		return *this;
	}
	Vector3<Type>& operator/=(const Vector3<Type>& v)
	{
		this->x /= v.x;
		this->y /= v.y;
		this->z /= v.z;
		return *this;
	}
	Vector3<Type>& operator=(const Vector3<Type>& v)
	{
		this->x = v.x;
		this->y = v.y;
		this->z = v.z;
		return *this;
	}

	Type magnitude() const { return sqrt(x*x + y * y + z * z); }
	Type sqrMagnitude() const { return x * x + y * y + z * z; }

	Vector3<Type> normalized() const {
		Type mag = magnitude();
		return Vector3<Type>(x / mag, y / mag, z / mag);
	}
	Vector3<Type>& normalize() { return *this /= magnitude(); }

	static Vector3<Type> Zero() { return Vector3<Type>(0, 0, 0); }
	static Vector3<Type> One() { return Vector3<Type>(1, 1, 1); }
};

template <typename Type>
Type Dot(Vector3<Type> v1, Vector3<Type> v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

template <typename Type>
Vector3<Type> Cross(Vector3<Type> v1, Vector3<Type> v2)
{
	return Vector3<Type>(
		v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x
		);
}

template <typename Type>
Vector3<Type> Reflect(const Vector3<Type>& v, const Vector3<Type>& n)
{
	return v - 2 * Dot<Type>(v, n) * n;
}


template <typename Type>
bool Refract(const Vector3<Type>& v, const Vector3<Type>& n, Type ni_over_nt, Vector3<Type>& refracted)
{
	Vector3<Type> uv = v.normalized();
	float dt = Dot(uv, n);
	float discriminant = 1.f - ni_over_nt * ni_over_nt * (1 - dt * dt);
	if (discriminant > 0)
	{
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return true;
	}
	else
		return false;
}

template <typename Type>
bool operator==(const Vector3<Type>& v1, const Vector3<Type>& v2)
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z;
}

template <typename Type>
bool operator!=(const Vector3<Type>& v1, const Vector3<Type>& v2)
{
	return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z;
}

template <typename Type>
bool operator<(const Vector3<Type>& v1, const Vector3<Type>& v2)
{
	return v1.x < v2.x && v1.y < v2.y && v1.z < v2.z;
}

template <typename Type>
bool operator<=(const Vector3<Type>& v1, const Vector3<Type>& v2)
{
	return v1.x <= v2.x && v1.y <= v2.y && v1.z <= v2.z;
}

template <typename Type>
bool operator>(const Vector3<Type>& v1, const Vector3<Type>& v2)
{
	return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z;
}

template <typename Type>
bool operator>=(const Vector3<Type>& v1, const Vector3<Type>& v2)
{
	return v1.x >= v2.x && v1.y >= v2.y && v1.z >= v2.z;
}

template <typename Type>
Vector3<Type> operator+(Vector3<Type> v1, Vector3<Type> v2)
{
	return Vector3<Type>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

template <typename Type>
Vector3<Type> operator+(Type t, Vector3<Type> v1)
{
	return Vector3<Type>(v1.x + t, v1.y + t, v1.z + t);
}

template <typename Type>
Vector3<Type> operator+(Vector3<Type> v1, Type t)
{
	return Vector3<Type>(v1.x + t, v1.y + t, v1.z + t);
}

template <typename Type>
Vector3<Type> operator-(Vector3<Type> v1, Vector3<Type> v2)
{
	return Vector3<Type>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

template <typename Type>
Vector3<Type> operator-(Type t, Vector3<Type> v1)
{
	return Vector3<Type>(t - v1.x, t - v1.y, t - v1.z);
}

template <typename Type>
Vector3<Type> operator-(Vector3<Type> v1, Type t)
{
	return Vector3<Type>(v1.x - t, v1.y - t, v1.z - t);
}

template <typename Type>
Vector3<Type> operator*(Vector3<Type> v, Type t)
{
	return Vector3<Type>(v.x * t, v.y * t, v.z * t);
}

template <typename Type>
Vector3<Type> operator*(Vector3<Type> v1, Vector3<Type> v2)
{
	return Vector3<Type>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

template <typename Type>
Vector3<Type> operator*(Type t, Vector3<Type> v)
{
	return Vector3<Type>(v.x * t, v.y * t, v.z * t);
}

template <typename Type>
Vector3<Type> operator/(const Vector3<Type>& v, Type t)
{
	return Vector3<Type>(v.x / t, v.y / t, v.z / t);
}

template <typename Type>
Vector3<Type> operator/(Type t, const Vector3<Type>& v)
{
	return Vector3<Type>(t / v.x, t / v.y, t / v.z);
}

template <typename Type>
Vector3<Type> operator/(const Vector3<Type>& v1, const Vector3<Type>& v2)
{
	return Vector3<Type>(v1.x / v2.x, v1.x / v2.y, v1.z / v2.z);
}

template <typename Type>
std::istream& operator>>(std::istream &i, Vector3<Type>& v)
{
	i >> v.x >> v.y >> v.z;
	return i;
}

template <typename Type>
std::ostream& operator<<(std::ostream &o, Vector3<Type>& v)
{
	o << v.x << ' ' << v.y << ' ' << v.z;
	return o;
}

#define MachineEpsilon (std::numeric_limits<float>::epsilon() * 0.5)

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

template <typename T>
int MaxDimension(const Vector3<T> &v) {
	return (v.x > v.y) ? ((v.x > v.z) ? 0 : 2) : ((v.y > v.z) ? 1 : 2);
}

template <typename T>
inline Vector3<T> Min(const Vector3<T> &p1, const Vector3<T> &p2) {
	return Vector3<T>(MIN(p1.x, p2.x), MIN(p1.y, p2.y), MIN(p1.z, p2.z));
}

template <typename T>
inline Vector3<T> Max(const Vector3<T> &p1, const Vector3<T> &p2) {
	return Vector3<T>(MAX(p1.x, p2.x), MAX(p1.y, p2.y), MAX(p1.z, p2.z));
}

template <typename T>
inline Vector3<T> Abs(const Vector3<T> &v) {
	return Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

template <typename T>
inline Vector3<T> Permute(const Vector3<T> &v, int x, int y, int z) {
	return Vector3<T>(v[x], v[y], v[z]);
}

template <typename T>
inline T MaxComponent(const Vector3<T> &v) {
	return MAX(v.x, MAX(v.y, v.z));
}

template <typename T>
inline const Vector3<T> Clamp(const Vector3<T> &val, const Vector3<T>& min, const Vector3<T>& max) {
	return Vector3<T>(Clamp(val.x, min.x, max.x), Clamp(val.y, min.y, max.y), Clamp(val.z, min.z, max.z));
}

template <typename T>
inline const Vector3<T> Clamp(const Vector3<T> &val, T min, const Vector3<T>& max) {
	return Vector3<T>(Clamp(val.x, min, max.x), Clamp(val.y, min, max.y), Clamp(val.z, min, max.z));
}

template <typename T>
inline const Vector3<T> Clamp(const Vector3<T> &val, const Vector3<T>& min, T max) {
	return Vector3<T>(Clamp(val.x, min.x, max), Clamp(val.y, min.y, max), Clamp(val.z, min.z, max));
}

template <typename T>
inline const Vector3<T> Clamp(const Vector3<T> &val, T min, T max) {
	return Vector3<T>(Clamp(val.x, min, max), Clamp(val.y, min, max), Clamp(val.z, min, max));
}

template <>
inline Vector3<float> Min(const Vector3<float> &p1, const Vector3<float> &p2) {
	return Vector3<float>(fmin(p1.x, p2.x), fmin(p1.y, p2.y), fmin(p1.z, p2.z));
}

template <>
inline Vector3<float> Max(const Vector3<float> &p1, const Vector3<float> &p2) {
	return Vector3<float>(fmax(p1.x, p2.x), fmax(p1.y, p2.y), fmax(p1.z, p2.z));
}

template <>
inline Vector3<float> Abs(const Vector3<float> &v) {
	return Vector3<float>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

inline Vector3<float> Sqrtv(const Vector3<float> &v) {
	return Vector3<float>(std::sqrt(v.x), std::sqrt(v.y), std::sqrt(v.z));
}

inline Vector3<double> Sqrtv(const Vector3<double> &v) {
	return Vector3<double>(std::sqrt(v.x), std::sqrt(v.y), std::sqrt(v.z));
}

inline bool IsNan(const Vector3<float> &v) {
	return isnan(v.x) || isnan(v.y) || isnan(v.z);
}
inline bool IsNan(const Vector3<double> &v) {
	return isnan(v.x) || isnan(v.y) || isnan(v.z);
}
inline bool IsNan(const Vector3<long double> &v) {
	return isnan(v.x) || isnan(v.y) || isnan(v.z);
}

template <>
inline float MaxComponent(const Vector3<float> &v) {
	return fmax(v.x, fmax(v.y, v.z));
}

typedef Vector3<float>				Vector3f;
typedef Vector3<double>				Vector3lf;
typedef Vector3<long double>		Vector3qf;
typedef Vector3<int>				Vector3i;
typedef Vector3<unsigned int>		Vector3u;

inline Vector3i Round(const Vector3f& vf)
{
#ifdef __CUDA_ARCH__
	return Vector3i(roundf(vf.x), roundf(vf.y), roundf(vf.z));
#else
	return Vector3i((int)round(vf.x), (int)round(vf.y), (int)round(vf.z));
#endif
}

inline Vector3f Lerp(const Vector3f& start, const Vector3f& end, float norm)
{
	return (1.f - norm) * start + end * norm;
}
inline Vector3lf Lerp(const Vector3lf& start, const Vector3lf& end, double norm)
{
	return (1. - norm) * start + end * norm;
}
inline Vector3qf Lerp(const Vector3qf& start, const Vector3qf& end, long double norm)
{
	return (1.L - norm) * start + end * norm;
}

template <typename Type>
struct Vector4
{
	union
	{
		struct
		{
			Type x;
			Type y;
			Type z;
			Type w;
		};
		Type dataByType[4];
		byte data[4 * sizeof(Type)];
	};

	Vector4() : x(0), y(0), z(0), w(0) { }
	Vector4(Type x, Type y, Type z, Type w) : x(x), y(y), z(z), w(w) {}
	Vector4(const Vector2<Type>& v0, const Vector2<Type>& v1) : x(v0.x), y(v0.y), z(v1.x), w(v1.y) {}
	Vector4(const Vector2<Type>& v, const Type& o0, const Type& o1) : x(v.x), y(v.y), z(o0), w(o1) {}
	Vector4(const Type& o0, const Vector2<Type>& v, const Type& o1) : x(o0), y(v.x), z(v.y), w(o1) {}
	Vector4(const Type& o0, const Type& o1, const Vector2<Type>& v) : x(o0), y(o1), z(v.x), w(v.y) {}
	Vector4(const Vector3<Type>& v, const Type& o) : x(v.x), y(v.y), z(v.z), w(o) {}
	Vector4(const Type& o, const Vector3<Type>& v) : x(o), y(v.x), z(v.y), w(v.z) {}
	Vector4(const Vector4<Type>& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}

	operator Vector2<Type>() const;
	operator Vector3<Type>() const;

	Type& operator[] (unsigned int i)
	{
		return dataByType[i & 0x03];
	}

	const Type& operator[] (unsigned int i) const
	{
		return dataByType[i & 0x03];
	}

	const Vector4<Type>& operator+() const { return *this; }
	Vector4<Type> operator-() const { return Vector4<Type>(-x, -y, -z, -w); }

	Vector4<Type>& operator+=(const Vector4<Type>& vec3)
	{
		this->x += vec3.x;
		this->y += vec3.y;
		this->z += vec3.z;
		this->w += vec3.w;
		return *this;
	}
	Vector4<Type>& operator-=(const Vector4<Type>& vec3)
	{
		this->x -= vec3.x;
		this->y -= vec3.y;
		this->z -= vec3.z;
		this->w -= vec3.w;
		return *this;
	}
	Vector4<Type>& operator*=(const float t)
	{
		this->x *= t;
		this->y *= t;
		this->z *= t;
		this->w *= t;
		return *this;
	}
	Vector4<Type>& operator/=(const float t)
	{
		this->x /= t;
		this->y /= t;
		this->z /= t;
		this->w /= t;
		return *this;
	}
	Vector4<Type>& operator/=(const Vector4<Type>& v)
	{
		this->x /= v.x;
		this->y /= v.y;
		this->z /= v.z;
		this->w /= v.w;
		return *this;
	}
	Vector4<Type>& operator*=(const Vector4<Type>& v)
	{
		this->x *= v.x;
		this->y *= v.y;
		this->z *= v.z;
		this->w *= v.w;
		return *this;
	}

	Type magnitude() const { return sqrt(x*x + y * y + z * z + w * w); }
	Type sqrMagnitude() const { return x * x + y * y + z * z + w * w; }

	Vector4<Type> normalized() const { return *this / magnitude(); }
	Vector4<Type>& normalize() { return *this /= magnitude(); }

	static Vector4<Type> Zero() { return Vector4<Type>(0, 0, 0, 0); }
	static Vector4<Type> One() { return Vector4<Type>(1, 1, 1, 1); }

	Vector3<Type>& GetVec3()
	{
		return Vector3<Type>(x, y, z);
	}
};

template <typename Type>
Type Dot(Vector4<Type> v1, Vector4<Type> v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z + v1.w * v2.w;
}

template <typename Type>
Vector4<Type> Reflect(const Vector4<Type>& v, const Vector4<Type>& n)
{
	return v - 2 * Dot<Type>(v, n) * n;
}

template <typename Type>
bool operator==(const Vector4<Type>& v1, const Vector4<Type>& v2)
{
	return v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w;
}

template <typename Type>
bool operator!=(const Vector4<Type>& v1, const Vector4<Type>& v2)
{
	return v1.x != v2.x || v1.y != v2.y || v1.z != v2.z || v1.w != v2.w;
}

template <typename Type>
bool operator<(const Vector4<Type>& v1, const Vector4<Type>& v2)
{
	return v1.x < v2.x && v1.y < v2.y && v1.z < v2.z && v1.w < v2.w;
}

template <typename Type>
bool operator<=(const Vector4<Type>& v1, const Vector4<Type>& v2)
{
	return v1.x <= v2.x && v1.y <= v2.y && v1.z <= v2.z && v1.w <= v2.w;
}

template <typename Type>
bool operator>(const Vector4<Type>& v1, const Vector4<Type>& v2)
{
	return v1.x > v2.x && v1.y > v2.y && v1.z > v2.z && v1.w > v2.w;
}

template <typename Type>
bool operator>=(const Vector4<Type>& v1, const Vector4<Type>& v2)
{
	return v1.x >= v2.x && v1.y >= v2.y && v1.z >= v2.z && v1.w >= v2.w;
}

template <typename Type>
Vector4<Type> operator+(Vector4<Type> v1, Vector4<Type> v2)
{
	return Vector4<Type>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
}

template <typename Type>
Vector4<Type> operator-(Vector4<Type> v1, Vector4<Type> v2)
{
	return Vector4<Type>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.z - v2.z);
}

template <typename Type>
Vector4<Type> operator*(Vector4<Type> v, float t)
{
	return Vector4<Type>(v.x * t, v.y * t, v.z * t, v.w * t);
}

template <typename Type>
Vector4<Type> operator*(Vector4<Type> v1, Vector4<Type> v2)
{
	return Vector4<Type>(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
}

template <typename Type>
Vector4<Type> operator*(float t, Vector4<Type> v)
{
	return Vector4<Type>(v.x * t, v.y * t, v.z * t, v.w * t);
}

template <typename Type>
Vector4<Type> operator/(Vector4<Type> v, Type t)
{
	return Vector4<Type>(v.x / t, v.y / t, v.z / t, v.w / t);
}

template <typename Type>
Vector4<Type> operator/(Type t, Vector4<Type> v)
{
	return Vector4<Type>(t / v.x, t / v.y, t / v.z, t / v.w);
}

template <typename Type>
Vector4<Type> operator/(const Vector4<Type>& v1, const Vector4<Type>& v2)
{
	return Vector4<Type>(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
}

template <typename Type>
std::istream& operator>>(std::istream &i, Vector4<Type>& v)
{
	i >> v.x >> v.y >> v.z >> v.w;
	return i;
}

template <typename Type>
std::ostream& operator<<(std::ostream &o, Vector4<Type>& v)
{
	o << v.x << ' ' << v.y << ' ' << v.z << ' ' << v.w;
	return o;
}

typedef Vector4<float>				Vector4f;
typedef Vector4<int>				Vector4i;
typedef Vector4<unsigned int>		Vector4u;

inline Vector4i Round(const Vector4f& vf)
{
	return Vector4i((int)(vf.x + 0.5f), (int)(vf.y + 0.5f), (int)(vf.z + 0.5f), (int)(vf.w + 0.5f));
}

template <typename Type>
Vector2<Type>::operator Vector3<Type>() const
{
	return Vector3<Type>(this->x, this->y, 0);
}

template <typename Type>
Vector2<Type>::operator Vector4<Type>() const
{
	return Vector4<Type>(this->x, this->y, 0, 0);
}

template <typename Type>
Vector3<Type>::operator Vector2<Type>() const
{
	return Vector2<Type>(this->x, this->y);
}

template <typename Type>
Vector3<Type>::operator Vector4<Type>() const
{
	return Vector4<Type>(this->x, this->y, this->z, 0);
}

template <typename Type>
Vector4<Type>::operator Vector2<Type>() const
{
	return Vector2<Type>(this->x, this->y);
}

template <typename Type>
Vector4<Type>::operator Vector3<Type>() const
{
	return Vector3<Type>(this->x, this->y, this->z);
}

struct Matrix4x4;
struct Quaternion
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

	Quaternion() { }
	Quaternion(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {  }
	Quaternion(const Quaternion& q) : x(q.x), y(q.y), z(q.z), w(q.w) { }

	Quaternion operator/(const float f)
	{
		ASSERT(f != 0);
		return Quaternion(x / f, y / f, z / f, w / f);
	}

	float Magnitude() const
	{
		return sqrt(x*x + y * y + z * z + w * w);
	}

	void Inverse()
	{
		x = -x;
		y = -y;
		z = -z;
	}

	Quaternion Inversed() const
	{
		return Quaternion(-x, -y, -z, w);
	}

	operator Vector3f() const
	{
		return Vector3f(x, y, z);
	}

	operator Vector4f() const
	{
		return Vector4f(x, y, z, w);
	}

	void Rotate(Vector3f& p) const;
	void ToMatrix(Matrix4x4& matrix) const;

	static Quaternion Identity() { return Quaternion(0, 0, 0, 1); }
};

inline Quaternion AngleAxis(float radian, const Vector3f& axis)
{
	if (IsNan(axis) || axis == Vector3f::Zero()) return Quaternion::Identity();

	float sin = sinf(radian * DEG2RAD / 2.f);
	return Quaternion(sin * axis.x, sin * axis.y, sin * axis.z, cosf(radian / 2.f * DEG2RAD));
}

inline void Inverse(Quaternion& q)
{
	q.x = -q.x;
	q.y = -q.y;
	q.z = -q.z;
}

inline Quaternion operator*(const Quaternion& q1, const Quaternion& q2)
{
	return Quaternion(
		q2.w*q1.x + q2.x * q1.w + q2.y * q1.z - q2.z * q1.y,
		q2.w*q1.y + q2.y * q1.w + q2.z * q1.x - q2.x * q1.z,
		q2.w*q1.z + q2.z * q1.w + q2.x * q1.y - q2.y * q1.x,
		q2.w*q1.w - q2.x * q1.x - q2.y * q1.y - q2.z * q1.z);
}

inline void Rotate(const Quaternion& q1, INOUT Vector3f& v)
{
	// implication : quaternion is normalized.
	Quaternion p(v.x, v.y, v.z, 0), q2(-q1.x, -q1.y, -q1.z, q1.w), q;

	q = q1 * p;
	q = q * q2;
	v.x = q.x;
	v.y = q.y;
	v.z = q.z;
}

inline Vector3f operator*(const Quaternion& q1, const Vector3f& v1)
{
	Vector3f v = v1;
	Rotate(q1, v);
	return v;
}

inline Vector3f& operator*(const Quaternion& q1, Vector3f& v1)
{
	Rotate(q1, v1);
	return v1;
}

inline float Luminance(const Vector3f& v)
{
	return 0.212671f * v.x + 0.715160f * v.y + 0.072169f * v.z;
}

enum class EulerAngleOrder : int
{
	OrderXYZ,
	OrderXZY,
	OrderYZX,
	OrderYXZ,
	OrderZXY,
	OrderZYX,
};

struct Matrix4x4;
void EulerAngleToMatrix(const Vector3f& eulerAngle, const EulerAngleOrder order, Matrix4x4& matirx);
void EulerAngleToQuaternion(const Vector3f& eulerAngle, const EulerAngleOrder order, Quaternion& q);
void EulerAngleToQuaternion(const Vector3f& eulerAngle, const EulerAngleOrder order, Quaternion& q);

// ??-major matrix 4x4, https://en.wikipedia.org/wiki/Row-_and_column-major_order
struct Matrix4x4
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
		char data[64];
		float dataf[16];
	};

	Matrix4x4()
	{
		m00 = m10 = m20 = m30 = m01 = m11 = m12 = m13 = m02 = m12 = m22 = m32 = m03 = m13 = m23 = m33 = 0;
	}
	Matrix4x4(const Matrix4x4& mat) :
		m00(mat.m00), m10(mat.m10), m20(mat.m20), m30(mat.m30),
		m01(mat.m01), m11(mat.m11), m21(mat.m21), m31(mat.m31),
		m02(mat.m02), m12(mat.m12), m22(mat.m22), m32(mat.m32),
		m03(mat.m03), m13(mat.m13), m23(mat.m23), m33(mat.m33)
	{}
	Matrix4x4(const Vector4f& c0, const Vector4f& c1, const Vector4f& c2, const Vector4f& c3)
	{
		this->c0 = c0;
		this->c1 = c1;
		this->c2 = c2;
		this->c3 = c3;
	}


	Vector3f TransformPoint(const Vector3f& pt) const
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
	Vector3f TransformVector(const Vector3f& v) const
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

	static Matrix4x4 GetIdentity()
	{
		Matrix4x4 m;
		m.m00 = m.m11 = m.m22 = m.m33 = 1;
		return m;
	}

	float operator [](int index)
	{
		return dataf[index];
	}

	static Matrix4x4 FromTRS(Vector3f translate, Quaternion r, Vector3f scale)
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

	static Matrix4x4 FromTRS(Vector3f translate, Vector3f eulerAngle, EulerAngleOrder order, Vector3f scale)
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
};

struct Ray
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

	Ray() { }
	Ray(const Ray& r) : origin(r.origin), direction(r.direction) { }
	Ray(const Vector3f& o, const Vector3f& d) : origin(o), direction(d) { }

	Ray& operator=(const Ray& r)
	{
		this->origin = r.origin;
		this->direction = r.direction;
		return *this;
	}

	Vector3f GetPosAt(float t) const
	{
		return origin + direction * t;
	}

	Ray Inversed() const
	{
		return Ray(origin, -direction);
	}
};

struct Bounds
{
	union
	{
		struct
		{
			Vector3f				center;
			Vector3f				extents;
		};
		byte data[24];
	};

	Bounds() : center(0, 0, 0), extents(0, 0, 0) { }
	Bounds(const Bounds& b) : center(b.center), extents(b.extents) { }
	Bounds(const Vector3f& c, const Vector3f& e) : center(c), extents(e) { }

	Bounds& operator=(const Bounds& v)
	{
		this->center = v.center;
		this->extents = v.extents;
		return *this;
	}

	bool Intersect(const Ray& r) const
	{
		// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
		// r.dir is unit direction vector of ray
		Vector3f dir_frac = 1.0f / r.direction;
		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// r.org is origin of ray
		Vector3f	smp = center - extents,
			bgp = center + extents;
		float t1 = (smp.x - r.origin.x) * dir_frac.x;
		float t2 = (bgp.x - r.origin.x) * dir_frac.x;
		float t3 = (smp.y - r.origin.y) * dir_frac.y;
		float t4 = (bgp.y - r.origin.y) * dir_frac.y;
		float t5 = (smp.z - r.origin.z) * dir_frac.z;
		float t6 = (bgp.z - r.origin.z) * dir_frac.z;

		float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
		float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));

		// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
		// if tmin > tmax, ray doesn't intersect AABB		
		return tmax >= 0 && tmin <= tmax;
	}

	bool Intersect(const Ray& r, float& t) const
	{
		// https://gamedev.stackexchange.com/questions/18436/most-efficient-aabb-vs-ray-collision-algorithms
		// r.dir is unit direction vector of ray
		Vector3f dir_frac = 1.0f / r.direction;
		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// r.org is origin of ray
		Vector3f	smp = center - extents,
			bgp = center + extents;
		float t1 = (smp.x - r.origin.x) * dir_frac.x;
		float t2 = (bgp.x - r.origin.x) * dir_frac.x;
		float t3 = (smp.y - r.origin.y) * dir_frac.y;
		float t4 = (bgp.y - r.origin.y) * dir_frac.y;
		float t5 = (smp.z - r.origin.z) * dir_frac.z;
		float t6 = (bgp.z - r.origin.z) * dir_frac.z;

		float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
		float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));

		// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
		if (tmax < 0)
		{
			t = tmax;
			return false;
		}

		// if tmin > tmax, ray doesn't intersect AABB
		if (tmin > tmax)
		{
			t = tmax;
			return false;
		}

		t = tmin;
		return true;
	}

	void Union(const Bounds& b)
	{
		Vector3f
			min = Min(b.center - b.extents, center - extents),
			max = Max(b.center + b.extents, center + extents);

		center = (max + min) / 2.f;
		extents = (max - min) / 2.f;
	}

};

inline Bounds Union(const Bounds& b0, const Bounds& b1)
{
	Vector3f
		min = Min(b0.center - b0.extents, b1.center - b1.extents),
		max = Max(b0.center + b0.extents, b1.center + b1.extents);

	return Bounds((min + max) / 2.f, (max - min) / 2.f);
}

inline void Union(const Bounds& b0, const Bounds& b1, Bounds& b)
{
	Vector3f
		min = Min(b0.center - b0.extents, b1.center - b1.extents),
		max = Max(b0.center + b0.extents, b1.center + b1.extents);

	b = Bounds((max + min) / 2.f, (max - min) / 2.f);
}

inline Vector3f ToFrameFromBasis(const Vector3f& v, const Vector3f& t, const Vector3f& b, const Vector3f& n)
{
	return Vector3f(Dot(v, t), Dot(v, b), Dot(v, n));
}

inline Vector3f ToBasisFromFrame(const Vector3f& v, const Vector3f& t, const Vector3f& b, const Vector3f& n)
{
	return
		Vector3f(
			t.x * v.x + b.x * v.y + n.x * v.z,
			t.y * v.x + b.y * v.y + n.y * v.z,
			t.z * v.x + b.z * v.y + n.z * v.z);
}

// TODO:: dq helper
struct DQ
{
	Quaternion real;
	Quaternion dual;
};

// TODO:: TRS helper
struct TRS
{
	Vector3f translate;
	Quaternion rotation;
	Vector3f scale;
};

inline void TRSToAffine(const TRS& trs, Matrix4x4& mat)
{
	mat = Matrix4x4::FromTRS(trs.translate, trs.rotation, trs.scale);
}
inline void AffineToTRS(const Matrix4x4& mat, TRS& trs)
{
	// TODO:: affine to trs
}
inline void TRSToDQ(const TRS& trs, DQ& dq)
{
	// TODO:: trs to dq
}
inline void DQToTRS(const DQ& dq, TRS& trs)
{
	// TODO:: dq to trs
}