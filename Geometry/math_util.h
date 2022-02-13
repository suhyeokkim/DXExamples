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
DECLSPEC_DLL uint32 Clamp(uint32 val, uint32 min, uint32 max);
DECLSPEC_DLL int64 Clamp(int64 val, int64 min, int64 max);
DECLSPEC_DLL uint64 Clamp(uint64 val, uint64 min, uint64 max);
DECLSPEC_DLL float Clamp(float val, float min, float max);
DECLSPEC_DLL dfloat Clamp(dfloat val, dfloat min, dfloat max);
DECLSPEC_DLL efloat Clamp(efloat val, efloat min, efloat max);

DECLSPEC_DLL float Luminance(float r, float g, float b);
