#include "transforms.h"

Matrix4x4 ProjectionPerspectiveLH(const Projection& proj)
{
    return ProjectionPerspectiveLH(proj.fov, proj.aspect, proj.nearZ, proj.farZ);
}

Matrix4x4 ProjectionPerspectiveLH(float fovAngleY, float aspectYToX, float nearZ, float farZ)
{
    auto cosFov = cos(fovAngleY + 0.5f);
    auto sinFov = sin(fovAngleY + 0.5f);

    auto aspectY = cosFov / sinFov;
    auto aspectX = aspectY / aspectYToX;
    auto range = farZ / (farZ - nearZ);

    auto m = Matrix4x4();

    /*
        c/s  0      0            0
        0    ay/c*s 0            0
        0    0      f/(f-n)      0
        0    0      -(f*n)/(f-n) 0
    */

    m.dataf[0] = aspectX;
    m.dataf[1 * 4 + 1] = aspectY;
    m.dataf[2 * 4 + 2] = range;
    m.dataf[2 + 4 + 3] = -range * nearZ;

    return m;
}

void TranslateTo(const Vector3f& t, Matrix4x4& mat)
{
    /*
        ?    ?    ?    t.x
        ?    ?    ?    t.y
        ?    ?    ?    t.z
        ?    ?    ?    ?
    */

    mat.c3.x = t.x;
    mat.c3.y = t.y;
    mat.c3.z = t.z;
}

void RotateTo(const Vector3f& forwardUM, const Vector3f& upUM, Matrix4x4& mat)
{
    auto fn = forwardUM.normalized();
    auto rn = Cross(fn, upUM.normalized());
    auto un = Cross(rn, fn);

    /*
        rn.x un.x fn.x ?
        rn.y un.y fn.y ?
        rn.z un.z fn.z ?
        ?    ?    ?    ?
    */

    mat.c0.x = rn.x;
    mat.c1.x = rn.y;
    mat.c2.x = rn.z;

    mat.c0.y = un.x;
    mat.c1.y = un.y;
    mat.c2.y = un.z;

    mat.c0.z = fn.x;
    mat.c1.z = fn.y;
    mat.c2.z = fn.z;
}

// https://stackoverflow.com/questions/52413464/look-at-quaternion-using-up-vector/52551983#52551983
void RotateTo(const Vector3f& forwardUM, const Vector3f& upUM, Quaternion& q)
{
    auto fn = forwardUM.normalized();
    auto rn = Cross(fn, upUM.normalized());
    auto un = Cross(rn, fn);

    /*
        rn.x un.x fn.x ?
        rn.y un.y fn.y ?
        rn.z un.z fn.z ?
        ?    ?    ?    ?
    */

    auto trace = rn.x + un.y + fn.z;
    if (trace > 0) {
        auto s = 0.5f / sqrtf(trace + 1.0f);
        q.w = 0.25f / s;
        q.x = (un.z - fn.y) * s;
        q.y = (fn.x - rn.x) * s;
        q.z = (un.z - fn.y) * s;
    } else {
        // trace maximum
        if (rn.x > un.y && rn.x > fn.z) {
            auto s = 2.0f * sqrtf(1.0f + rn.x - un.y - fn.z);
            q.w = (un.z - fn.y) / s;
            q.x = 0.25 * s;
            q.y = (un.x + rn.y) / s;
            q.z = (fn.x + rn.z) / s;
        } else if (un.y > fn.z) {
            auto s = 2.0f * sqrtf(1.0f + un.y - rn.x - fn.z);
            q.w = (fn.x - rn.z) / s;
            q.z = (un.x + rn.y) / s;
            q.y = 0.25 * s;
            q.z = (fn.y + un.z) / s;
        } else {
            auto s = 2.0f * sqrtf(1.0f + fn.z - rn.x - un.y);
            q.w = (rn.y - un.x) / s;
            q.x = (fn.x + rn.z) / s;
            q.y = (fn.y + un.z) / s;
            q.z = 0.25 * s;
        }
    }
}

#pragma region Matrix4x4

// cardan angle(Tait–Bryan angles), https://link.springer.com/content/pdf/bbm%3A978-1-4612-3502-6%2F1.pdf
void EulerAngleToMatrix(const Vector3f& eulerAngle, const EulerAngleOrder order, Matrix4x4& matrix)
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

        matrix.dataf[2] = -sx;
        matrix.dataf[6] = sx * cy;
        matrix.dataf[10] = cx * cy;
        break;
    case EulerAngleOrder::OrderXZY:
        matrix.dataf[0] = cy * cz;
        matrix.dataf[4] = sx * sy - cx * cy * sz;
        matrix.dataf[8] = sx * cy * sz + cx * sy;

        matrix.dataf[1] = sz;
        matrix.dataf[5] = cx * cz;
        matrix.dataf[9] = -sx * cz;

        matrix.dataf[2] = -sy * cz;
        matrix.dataf[6] = cx * sy * sz + sx * cy;
        matrix.dataf[10] = cx * cy - sx * sy * sz;
        break;
    case EulerAngleOrder::OrderYXZ:
        matrix.dataf[0] = cy * cz - sx * sy * sz;
        matrix.dataf[4] = -cx * sz;
        matrix.dataf[8] = sy * cz + sx * cy * sz;

        matrix.dataf[1] = cy * cz + sx * sy * cz;
        matrix.dataf[5] = cx * cz;
        matrix.dataf[9] = sy * sz - sx * cy * cx;

        matrix.dataf[2] = -cx * sy;
        matrix.dataf[6] = sx;
        matrix.dataf[10] = cx * cy;
        break;
    case EulerAngleOrder::OrderYZX:
        matrix.dataf[0] = cy * sz;
        matrix.dataf[4] = -sz;
        matrix.dataf[8] = sy * cz;

        matrix.dataf[1] = cx * cy * sz + sx * sy;
        matrix.dataf[5] = cx * cz;
        matrix.dataf[9] = cx * sy * sz - sx * cy;

        matrix.dataf[2] = sx * cy * sz - cx * sy;
        matrix.dataf[6] = sx * cz;
        matrix.dataf[10] = sx * sy * sz - cx * cy;
        break;
    case EulerAngleOrder::OrderZYX:
        matrix.dataf[0] = cy * cx;
        matrix.dataf[4] = -cy * sz;
        matrix.dataf[8] = sy;

        matrix.dataf[1] = cx * sy + sx * sy * cx;
        matrix.dataf[5] = cx * cz - sz * sy * sz;
        matrix.dataf[9] = -sx * cy;

        matrix.dataf[2] = sx * sz - cx * sy * cz;
        matrix.dataf[6] = sx * cz + cx * sy * sz;
        matrix.dataf[10] = cx * cy;
        break;
    case EulerAngleOrder::OrderZXY:
        matrix.dataf[0] = cy * cz + sx * sy * sz;
        matrix.dataf[1] = cx * sz;
        matrix.dataf[2] = sx * cy * sz - sy * cz;

        matrix.dataf[4] = -cy * sz;
        matrix.dataf[5] = cx * cz;
        matrix.dataf[6] = sy * sz + sx * cy * cz;

        matrix.dataf[8] = cx * sy;
        matrix.dataf[9] = -sx;
        matrix.dataf[10] = cx * cy;
        break;
    }

    matrix.dataf[3] = 0;
    matrix.dataf[7] = 0;
    matrix.dataf[11] = 0;

    matrix.dataf[12] = 0;
    matrix.dataf[13] = 0;
    matrix.dataf[14] = 0;
    matrix.dataf[15] = 1;
}

void EulerAngleToQuaternion(const Vector3f& eulerAngle, const EulerAngleOrder order, Quaternion& q)
{
}

#pragma endregion

Matrix4x4 TRSToMatrix(const TRS& trs)
{
    return TRSToMatrix(trs.translate, trs.rotation, trs.scale);
}

Matrix4x4 TRSToMatrix(Vector3f translate, Quaternion rotation, Vector3f scale)
{
    Matrix4x4 m0;
    ToAffineMatrix(rotation, m0);

    m0.dataf[0] *= scale.x;
    m0.dataf[1] *= scale.x;
    m0.dataf[2] *= scale.x;
    // m0.dataf[3] 
    m0.dataf[4] *= scale.y;
    m0.dataf[5] *= scale.y;
    m0.dataf[6] *= scale.y;
    // m0.dataf[7] 
    m0.dataf[8] *= scale.z;
    m0.dataf[9] *= scale.z;
    m0.dataf[10] *= scale.z;
    // m0.dataf[11] 
    m0.dataf[12] = translate.x;
    m0.dataf[13] = translate.y;
    m0.dataf[14] = translate.z;
    // m0.dataf[15] 

    return m0;
}

Matrix4x4 TRSToMatrix(Vector3f translate, Vector3f eulerAngle, EulerAngleOrder order, Vector3f scale)
{
    Matrix4x4 m0;
    EulerAngleToMatrix(eulerAngle, order, m0);

    m0.dataf[0] *= scale.x;
    m0.dataf[1] *= scale.x;
    m0.dataf[2] *= scale.x;
    // m0.dataf[3] 
    m0.dataf[4] *= scale.y;
    m0.dataf[5] *= scale.y;
    m0.dataf[6] *= scale.y;
    // m0.dataf[7] 
    m0.dataf[8] *= scale.z;
    m0.dataf[9] *= scale.z;
    m0.dataf[10] *= scale.z;
    // m0.dataf[11] 
    m0.dataf[12] = translate.x;
    m0.dataf[13] = translate.y;
    m0.dataf[14] = translate.z;
    // m0.dataf[15] 

    return m0;
}

Quaternion AngleAxis(float radian, const Vector3f& axis)
{
    if (axis.IsNan() || axis == Vector3f::Zero()) return Quaternion::Identity();

    float sin = sinf(radian * DEG2RAD / 2.f);
    return Quaternion(sin * axis.x, sin * axis.y, sin * axis.z, cosf(radian / 2.f * DEG2RAD));
}

void Rotate(const Quaternion& q, Vector3f& p)
{
    float num01 = q.x * 2, num02 = q.y * 2, num03 = q.z * 2;
    float num04 = q.x * num01, num05 = q.y * num02, num06 = q.z * num03;
    float num07 = q.x * num02, num08 = q.x * num03, num09 = q.y * num03;
    float num10 = q.w * num01, num11 = q.w * num02, num12 = q.w * num03;

    Vector3f result;
    result.x = (1 - (num05 + num06)) * p.x + (num07 - num12) * p.y + (num08 + num11) * p.z;
    result.y = (num07 + num12) * p.x + (1 - (num04 + num06)) * p.y + (num09 - num10) * p.z;
    result.z = (num08 - num11) * p.x + (num09 + num10) * p.y + (1 - (num04 + num05)) * p.z;
    p = result;
}

// https://neoplanetz.tistory.com/entry/CV-%EC%BF%BC%ED%84%B0%EB%8B%88%EC%96%B8%EC%9D%84-%EB%A1%9C%ED%85%8C%EC%9D%B4%EC%85%98-%EB%A7%A4%ED%8A%B8%EB%A6%AD%EC%8A%A4%EB%A1%9C-%EB%B3%80%ED%99%98-Quaternion-to-Rotation-Matrix
void ToAffineMatrix(const Quaternion& q, Matrix4x4& matrix)
{
    float xx = q.x * q.x, yy = q.y * q.y, zz = q.z * q.z;
    float xy = q.x * q.y, yz = q.y * q.z, zx = q.z * q.x;
    float xw = q.x * q.w, yw = q.y * q.w, zw = q.z * q.w;

    matrix.dataf[0] = 1 - 2 * (yy + zz);
    matrix.dataf[4] = 2 * (xy - zw);
    matrix.dataf[8] = 2 * (zx + yw);
    matrix.dataf[3] = 0;

    matrix.dataf[1] = 2 * (xy + zw);
    matrix.dataf[5] = 1 - 2 * (xx + zz);
    matrix.dataf[9] = 2 * (yz - xw);
    matrix.dataf[7] = 0;

    matrix.dataf[2] = 2 * (zx - yw);
    matrix.dataf[6] = 2 * (yz + xw);
    matrix.dataf[10] = 1 - 2 * (xx + yy);
    matrix.dataf[11] = 0;

    matrix.dataf[12] = 0;
    matrix.dataf[13] = 0;
    matrix.dataf[14] = 0;
    matrix.dataf[15] = 1;
}

Vector3f ToFrameFromBasis(const Vector3f& v, const Vector3f& t, const Vector3f& b, const Vector3f& n)
{
    return Vector3f(Dot(v, t), Dot(v, b), Dot(v, n));
}

Vector3f ToBasisFromFrame(const Vector3f& v, const Vector3f& t, const Vector3f& b, const Vector3f& n)
{
    return Vector3f(
        t.x * v.x + b.x * v.y + n.x * v.z,
        t.y * v.x + b.y * v.y + n.y * v.z,
        t.z * v.x + b.z * v.y + n.z * v.z
    );
}

void TRSToAffine(const TRS& trs, Matrix4x4& mat)
{
    mat = TRSToMatrix(trs.translate, trs.rotation, trs.scale);
}

void AffineToTRS(const Matrix4x4& mat, TRS& trs)
{
    // TODO:: affine to trs
}

void TRSToDQ(const TRS& trs, DQ& dq)
{
    // TODO:: trs to dq
}

void DQToTRS(const DQ& dq, TRS& trs)
{
    // TODO:: dq to trs
}