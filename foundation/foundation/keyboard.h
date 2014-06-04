#pragma once

#ifdef _WIN32
	#include <Windows.h>
#endif

namespace bowtie
{
namespace keyboard
{

#ifdef _WIN32
	enum Key
	{
		Up = VK_UP,
		Down = VK_DOWN,
		Left = VK_LEFT,
		Right = VK_RIGHT,
		A = 0x41,
		Z = 0x5A
	};

#endif

}
}