#pragma once

#if defined(_MSC_VER) && !defined(_ALLOW_KEYWORD_MACROS)
	#define _ALLOW_KEYWORD_MACROS
#endif
	
#if !defined(alignof)
	#define alignof(x) __alignof(x)
#endif

namespace bowtie
{
typedef signed char int8;
typedef short int16;
typedef int int32;
typedef long long int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long long uint64;
typedef float real32;
typedef double real64;
typedef wchar_t wchar;
static const uint32 NotInitialized = 0;
}
