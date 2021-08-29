#pragma once

#include <windows.h>

struct WindowSetting
{
	const WCHAR* windowName;
	uint windowWidth;
	uint windowHeight;
	uint maxFrameRate;

	WindowSetting() : windowName(nullptr), windowWidth(0), windowHeight(0), maxFrameRate(0)
	{

	}

	WindowSetting(const char* mbWindowName, uint width, uint height, uint maxFrameRate)
	{
		int len = strlen(mbWindowName);
		WCHAR* newWindowName = (WCHAR*)malloc(sizeof(WCHAR) * (len + 1));
		mbsrtowcs(newWindowName, &mbWindowName, len, nullptr);
		windowName = newWindowName;
		windowWidth = width;
		windowHeight = height;
		this->maxFrameRate = maxFrameRate;
	}
};
