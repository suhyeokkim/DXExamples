#pragma once

#ifdef EXPORT_COMMON_DLL
#define DECLSPEC_DLL __declspec(dllexport)
#else
#define DECLSPEC_DLL __declspec(dllimport)
#endif