#pragma once

#include <stdint.h>

#if defined(_MSC_VER) && !defined(_ALLOW_KEYWORD_MACROS)
	#define _ALLOW_KEYWORD_MACROS
#endif
	
#if !defined(alignof)
	#define alignof(x) __alignof(x)
#endif

namespace bowtie
{
	static const unsigned handle_not_initialized = 0;
}