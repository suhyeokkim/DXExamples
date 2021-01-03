#include "datatypes.h"

float RandomFloat(float min, float max)
{
	// this  function assumes max > min, you may want 
	// more robust error checking for a non-debug build
	assert(max > min);
	float random = ((float)rand()) / (float)RAND_MAX;

	// generate (in your case) a float between 0 and (4.5-.78)
	// then add .78, giving you a float between .78 and 4.5
	float range = max - min;
	return (random*range) + min;
}

void Quaternion::Rotate(Vector3f& p) const
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

// https://neoplanetz.tistory.com/entry/CV-%EC%BF%BC%ED%84%B0%EB%8B%88%EC%96%B8%EC%9D%84-%EB%A1%9C%ED%85%8C%EC%9D%B4%EC%85%98-%EB%A7%A4%ED%8A%B8%EB%A6%AD%EC%8A%A4%EB%A1%9C-%EB%B3%80%ED%99%98-Quaternion-to-Rotation-Matrix
void Quaternion::ToMatrix(Matrix4x4& matrix) const
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

	matrix.dataf[2] = 2 * (zx - yw);
	matrix.dataf[6] = 2 * (yz + xw);
	matrix.dataf[10] = 1 - 2 * (xx + yy);
	matrix.dataf[11] = 0;

	matrix.dataf[12] = 0;
	matrix.dataf[13] = 0;
	matrix.dataf[14] = 0;
	matrix.dataf[15] = 1;
}

// cardan angle(Tait–Bryan angles), https://link.springer.com/content/pdf/bbm%3A978-1-4612-3502-6%2F1.pdf
void EulerAngleToMatrix(const Vector3f& eulerAngle, const EulerAngleOrder order, Matrix4x4& matrix)
{
	float cx = cosf(eulerAngle.x * DEG2RAD), cy = cosf(eulerAngle.y * DEG2RAD), cz = cosf(eulerAngle.z * DEG2RAD);
	float sx = sinf(eulerAngle.x * DEG2RAD), sy = sinf(eulerAngle.y * DEG2RAD), sz = sinf(eulerAngle.z * DEG2RAD);

	switch (order)
	{
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
		matrix.dataf[4] = sx * sy - cx * cy*sz;
		matrix.dataf[8] = sx * cy*sz + cx * sy;

		matrix.dataf[1] = sz;
		matrix.dataf[5] = cx * cz;
		matrix.dataf[9] = -sx * cz;

		matrix.dataf[2] = -sy * cz;
		matrix.dataf[6] = cx * sy*sz + sx * cy;
		matrix.dataf[10] = cx * cy - sx * sy*sz;
		break;
	case EulerAngleOrder::OrderYXZ:
		matrix.dataf[0] = cy * cz - sx * sy*sz;
		matrix.dataf[4] = -cx * sz;
		matrix.dataf[8] = sy * cz + sx * cy*sz;

		matrix.dataf[1] = cy * cz + sx * sy*cz;
		matrix.dataf[5] = cx * cz;
		matrix.dataf[9] = sy * sz - sx * cy*cx;

		matrix.dataf[2] = -cx * sy;
		matrix.dataf[6] = sx;
		matrix.dataf[10] = cx * cy;
		break;
	case EulerAngleOrder::OrderYZX:
		matrix.dataf[0] = cy * sz;
		matrix.dataf[4] = -sz;
		matrix.dataf[8] = sy * cz;

		matrix.dataf[1] = cx * cy*sz + sx * sy;
		matrix.dataf[5] = cx * cz;
		matrix.dataf[9] = cx * sy*sz - sx * cy;

		matrix.dataf[2] = sx * cy*sz - cx * sy;
		matrix.dataf[6] = sx * cz;
		matrix.dataf[10] = sx * sy*sz - cx * cy;
		break;
	case EulerAngleOrder::OrderZYX:
		matrix.dataf[0] = cy * cx;
		matrix.dataf[4] = -cy * sz;
		matrix.dataf[8] = sy;

		matrix.dataf[1] = cx * sy + sx * sy*cx;
		matrix.dataf[5] = cx * cz - sz * sy*sz;
		matrix.dataf[9] = -sx * cy;

		matrix.dataf[2] = sx * sz - cx * sy*cz;
		matrix.dataf[6] = sx * cz + cx * sy*sz;
		matrix.dataf[10] = cx * cy;
		break;
	case EulerAngleOrder::OrderZXY:
		matrix.dataf[0] = cy * cz + sx * sy*sz;
		matrix.dataf[1] = cx * sz;
		matrix.dataf[2] = sx * cy*sz - sy * cz;

		matrix.dataf[4] = -cy * sz;
		matrix.dataf[5] = cx * cz;
		matrix.dataf[6] = sy * sz + sx * cy*cz;

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

#pragma endregion