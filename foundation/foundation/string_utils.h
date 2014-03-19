#pragma once

#include <cstring>

namespace bowtie
{
	
inline bool strequal(const char* str1, const char* str2)
{
	return strcmp(str1, str2) == 0;
}

inline unsigned strlen32(const char* str)
{
	return unsigned(strlen(str));
}

}