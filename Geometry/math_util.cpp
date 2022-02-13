#include "math_util.h"
#include "defined_macro.h"

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

#pragma endregion
