#pragma once

#include <cstring>

namespace bowtie
{
	
inline bool strequal(const char* str1, const char* str2)
{
	if (str1 == 0 && str2 == 0)
		return true;

	if ((str1 == 0 && str2 != 0) || (str1 != 0 && str2 == 0))
		return false; 

	return strlen(str1) == strlen(str2) && strcmp(str1, str2) == 0;
}

inline unsigned strlen32(const char* str)
{
	if (str == 0)
		return 0;

	return unsigned(strlen(str));
}

}