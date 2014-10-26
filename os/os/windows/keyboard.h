#pragma once
#ifdef _WIN32

#include <Windows.h>

namespace bowtie
{

namespace keyboard
{

#ifdef _WIN32
	enum Key
	{
		F5 = VK_F5,
		Up = VK_UP,
		Down = VK_DOWN,
		Left = VK_LEFT,
		Right = VK_RIGHT,
		A = 0x41,
		Z = 0x5A,
		R = 0x52,
		PageUp = VK_PRIOR,
		PageDown = VK_NEXT,
		Tilde = 0xDC
	};

#endif

}

}

#endif
