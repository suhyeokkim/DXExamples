#pragma once

#ifdef EXPORT_GEOMETRY_DLL
#define DECLSPEC_DLL __declspec(dllexport)
#else
#define DECLSPEC_DLL __declspec(dllimport)
#endif