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

typedef unsigned short         ushort;
typedef unsigned int           uint;
typedef signed long int        lint;
typedef unsigned long int      ulint;
typedef signed long long int   llint;
typedef unsigned long long int ullint;
typedef double                 dfloat;
typedef long double            efloat;
