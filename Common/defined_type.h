#pragma once

#if defined (DEBUG) | (_DEBUG)
#define _CRTDBG_MAP_ALLOC 
#endif

typedef char                   int8;
typedef unsigned char          uint8;
typedef short                  int16;
typedef unsigned short         uint16;
typedef int                    int32;
typedef unsigned int           uint32;
typedef signed long long int   int64;
typedef unsigned long long int uint64;
typedef double                 dfloat;
typedef long double            efloat;

#define scast                  static_cast
#define dcast                  dynamic_cast
#define rcast                  reinterpret_cast
#define ccast                  const_cast