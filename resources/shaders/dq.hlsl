#ifndef DUALQUATERNION_ARTHIMETIC
#define DUALQUATERNION_ARTHIMETIC

// UnityEngine.Quaternion operator*
float4 mulQxQ(float4 lhs, float4 rhs)
{
	return float4(
		lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.w * rhs.y + lhs.y * rhs.w + lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.w * rhs.z + lhs.z * rhs.w + lhs.x * rhs.y - lhs.y * rhs.x,
		lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z
		);
}

float4 conjugateQuaternion(float4 q)
{
	return float4(q.xyz * -1.0, q.w);
}

// https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion
float3 transformPositionByQ(float4 q, float3 p)
{
	//float3 u = float3(q.x, q.y, q.z);
	//float s = q.w;
	//return 2.0 * dot(u, p) * u + (s * s - dot(u, u)) * p + 2.0 * s * cross(u, p);

	float num = q.x * 2;
	float num2 = q.y * 2;
	float num3 = q.z * 2;
	float num4 = q.x * num;
	float num5 = q.y * num2;
	float num6 = q.z * num3;
	float num7 = q.x * num2;
	float num8 = q.x * num3;
	float num9 = q.y * num3;
	float num10 = q.w * num;
	float num11 = q.w * num2;
	float num12 = q.w * num3;

	float3 result;
	result.x = (1 - (num5 + num6)) * p.x + (num7 - num12) * p.y + (num8 + num11) * p.z;
	result.y = (num7 + num12) * p.x + (1 - (num4 + num6)) * p.y + (num9 - num10) * p.z;
	result.z = (num8 - num11) * p.x + (num9 + num10) * p.y + (1 - (num4 + num5)) * p.z;
	return result;
}

inline float3x3 quaternionToMatrix(float4 q)
{
	float3x3 rotationMatrix;

	return rotationMatrix;
}

struct DQ
{
	float4 real;
	float4 dual;
};

inline DQ multiply(DQ dq, float scalar)
{
	dq.real *= scalar;
	dq.dual *= scalar;

	return dq;
}

inline DQ add(DQ dq1, DQ dq2)
{
	DQ dq;

	dq.real = dq1.real + dq2.real;
	dq.dual = dq1.dual + dq2.dual;

	return dq;
}

inline DQ minus(DQ dq1, DQ dq2)
{
	DQ dq;

	dq.real = dq1.real - dq2.real;
	dq.dual = dq1.dual - dq2.dual;

	return dq;
}

DQ normalizeDQ(DQ dq)
{
	float len = length(dq.real);
	dq.real = dq.real / len;
	dq.dual = dq.dual / len;
    return dq;
}

DQ mulDQ(DQ dq1, DQ dq2)
{
	DQ dq;

	dq.real = mulQxQ(dq1.real, dq2.real);
	dq.dual = mulQxQ(dq1.dual, dq2.real) + mulQxQ(dq1.real, dq2.dual);

	return dq;
}

float3 translateFromDQ(DQ dq)
{
	return
		mulQxQ(
			dq.dual * 2,
			conjugateQuaternion(dq.real)
		).xyz;
}

float3 transformPositionByDQ(DQ dq, float3 pos)
{
	return translateFromDQ(dq) + transformPositionByQ(dq.real, pos);
}

float4x4 DQToMatrix(DQ dq)
{
	float4x4 convetedMatrix;
	float len2 = dot(dq.real, dq.real);
	float   xx = dq.real.x * dq.real.x, xy = dq.real.x * dq.real.y, xz = dq.real.x * dq.real.z, xw = dq.real.x * dq.real.w,
		yy = dq.real.y * dq.real.y, yz = dq.real.y * dq.real.z, yw = dq.real.y * dq.real.w,
		zz = dq.real.z * dq.real.z, zw = dq.real.z * dq.real.w;

	convetedMatrix[0][0] = 1.0 - 2.0 * yy - 2.0 * zz;
	convetedMatrix[0][1] = 2.0 * xy - 2.0 * zw;
	convetedMatrix[0][2] = 2.0 * xz + 2.0 * yw;

	convetedMatrix[1][0] = 2.0 * xy + 2 * zw;
	convetedMatrix[1][1] = 1.0 - 2.0 * xx - 2.0 * zz;
	convetedMatrix[1][2] = 2.0 * yz - 2.0 * xw;

	convetedMatrix[2][0] = 2.0 * xz - 2.0 * yw;
	convetedMatrix[2][1] = 2.0 * yz + 2.0 * xw;
	convetedMatrix[2][2] = 1.0 - 2.0 * xx - 2.0 * yy;

	float3 trans = translateFromDQ(dq);

	convetedMatrix[0][3] = trans.x;
	convetedMatrix[1][3] = trans.y;
	convetedMatrix[2][3] = trans.z;

	convetedMatrix[3][0] = 0;
	convetedMatrix[3][1] = 0;
	convetedMatrix[3][2] = 0;

	convetedMatrix /= len2;

	convetedMatrix[3][3] = 1.0;

	return convetedMatrix;
}
#endif
