#pragma once

#include <windows.h>

struct WindowSetting
{
	const WCHAR* windowName;
	uint windowWidth;
	uint windowHeight;
	uint maxFrameRate;
};
