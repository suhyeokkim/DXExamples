#pragma once

#include "symbols.h"
#include "math_util.h"
#include "units.h"

// left-handed coordinate

struct DECLSPEC_DLL DQ
{
    Quaternion real;
    Quaternion dual;
};

struct DECLSPEC_DLL TRS
{
    Vector3f   translate;
    Quaternion rotation;
    Vector3f   scale;
};

enum class DECLSPEC_DLL EulerAngleOrder : int
{
    OrderXYZ,
    OrderXZY,
    OrderYZX,
    OrderYXZ,
    OrderZXY,
    OrderZYX,
};

DECLSPEC_DLL void EulerAngleToMatrix(const Vector3f& eulerAngle, const EulerAngleOrder order, Matrix4x4& matirx);
DECLSPEC_DLL void EulerAngleToQuaternion(const Vector3f& eulerAngle, const EulerAngleOrder order, Quaternion& q);

DECLSPEC_DLL void TranslateTo(const Vector3f& translate, Matrix4x4& mat);

DECLSPEC_DLL void RotateTo(const Vector3f& forward, const Vector3f& up, Matrix4x4& mat);
DECLSPEC_DLL void RotateTo(const Vector3f& forward, const Vector3f& up, Quaternion& q);

DECLSPEC_DLL Matrix4x4 TRSToMatrix(const TRS& trs);
DECLSPEC_DLL Matrix4x4 TRSToMatrix(Vector3f translate, Quaternion rotation, Vector3f scale);
DECLSPEC_DLL Matrix4x4 TRSToMatrix(Vector3f translate, Vector3f eulerAngle, EulerAngleOrder order, Vector3f scale);

DECLSPEC_DLL void Rotate(const Quaternion& r, Vector3f& p);
DECLSPEC_DLL void ToAffineMatrix(const Quaternion& r, Matrix4x4& matrix);
DECLSPEC_DLL Quaternion AngleAxis(float radian, const Vector3f& axis);
DECLSPEC_DLL Vector3f ToFrameFromBasis(const Vector3f& v, const Vector3f& t, const Vector3f& b, const Vector3f& n);
DECLSPEC_DLL Vector3f ToBasisFromFrame(const Vector3f& v, const Vector3f& t, const Vector3f& b, const Vector3f& n);

DECLSPEC_DLL void TRSToAffine(const TRS& trs, Matrix4x4& mat);
DECLSPEC_DLL void AffineToTRS(const Matrix4x4& mat, TRS& trs);
DECLSPEC_DLL void TRSToDQ(const TRS& trs, DQ& dq);
DECLSPEC_DLL void DQToTRS(const DQ& dq, TRS& trs);

struct DECLSPEC_DLL Projection
{
    float fov;
    float aspect;
    float nearZ;
    float farZ;
};

DECLSPEC_DLL Matrix4x4 ProjectionPerspectiveLH(const Projection& proj);
DECLSPEC_DLL Matrix4x4 ProjectionPerspectiveLH(float fovAngleY, float aspectYToX, float nearZ, float farZ);