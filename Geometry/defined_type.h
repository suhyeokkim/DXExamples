#pragma once

#if defined (DEBUG) | (_DEBUG)
#define _CRTDBG_MAP_ALLOC 
#endif

#include <vector>
#include <cstdlib>
#include <cassert>
#include <windows.h>
#include <functional>
#include <chrono>

#define IN
#define OUT
#define INOUT
#define REF

typedef short                  int16;
typedef unsigned short         uint16;
typedef int                    int32;
typedef unsigned int           uint32;
typedef signed long long int   int64;
typedef unsigned long long int uint64;
typedef double                 dfloat;
typedef long double            efloat;
