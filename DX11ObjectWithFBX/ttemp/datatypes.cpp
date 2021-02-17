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