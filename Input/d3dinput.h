#pragma once

#include "symbols.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

// 마우스 : 2는 네개 이상의 버튼
// https://docs.microsoft.com/en-us/previous-versions/windows/desktop/ee418672(v=vs.85)
// 조이스틱 : 1,2 머임?
// https://docs.microsoft.com/ko-kr/previous-versions/windows/desktop/ee418631(v=vs.85)

enum class DECLSPEC_DLL InputDeviceFlag
{
	Keyboard				= 0x01,
	Mouse					= 0x02,
	ExtendMouse				= 0x04,
	Joystick				= 0x08,
	Joystick2				= 0x10,
};

DECLSPEC_DLL bool InitializeInput(HINSTANCE instacne, InputDeviceFlag flags);
DECLSPEC_DLL bool IsFailed(InputDeviceFlag flag);
