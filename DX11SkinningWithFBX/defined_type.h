#if defined (DEBUG) | (_DEBUG)
#define _CRTDBG_MAP_ALLOC 
#endif

#include <vector>
#include <cstdlib>
#include <cassert>
#include <windows.h>
#include <functional>

#pragma once

#define IN
#define OUT
#define INOUT
#define REF

typedef unsigned short ushort;
typedef signed char sbyte;
typedef unsigned char ubyte;
typedef signed int sint;
typedef unsigned int uint;
typedef signed long slong;
typedef unsigned long ulong;
typedef long long int lint;
typedef signed long long int slint;
typedef unsigned long long int ulint;

#include <chrono>

const long long int m2h = 60;
const long long int s2m = 60;
const long long int ms2s = 1000;
const long long int us2ms = 1000;
const long long int ns2us = 1000;

struct ProfileTime
{
	using nst = std::chrono::nanoseconds;
	using syst = std::chrono::system_clock;

	const char* string;
	std::chrono::system_clock::time_point start;

	ProfileTime(const char* string) : string(string)
	{
		start = syst::now();
	}
	~ProfileTime()
	{
		FILE* fp = stdout;
		nst ns = std::chrono::duration_cast<nst>(syst::now() - start);
		decltype(ns.count()) moded = ns.count();

		if (moded / (1000 * 1000) > 0)
			fprintf(fp, "%.2lfms ", (double)moded / (1000 * 1000));

		fputs("\n", fp);
	}
};
