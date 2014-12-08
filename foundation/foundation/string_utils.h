#pragma once

#include <cstring>
#include "array.h"

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

inline Array<char*> split(Allocator& allocator, const char* str, char delimiter)
{
	auto words = array::create<char*>(allocator);
	auto len_str = strlen(str);

	if (len_str == 0)
		return words;

	auto len = len_str + 1;
	unsigned current_word_len = 0;

	for (unsigned i = 0; i < len; ++i)
	{
		if (str[i] == delimiter || i == len - 1)
		{
			char* word = (char*)allocator.alloc_raw(current_word_len + 1);
			memcpy(word, str + i - current_word_len, current_word_len);
			word[current_word_len] = 0;
			array::push_back(words, word);
			current_word_len = 0;
		}
		else
			++current_word_len;
	}

	return words;
}

inline float float_from_str(const char* str)
{
	return (float)strtod(str, nullptr);
}

inline unsigned unsigned_from_str(const char* str)
{
	return (unsigned)strtoul(str, nullptr, 10);
}

inline char* copy_str(Allocator& allocator, const char* str, unsigned len)
{
	auto size = len + 1;
	auto new_str = (char*)allocator.alloc_raw(size);
	strcpy_s(new_str, size, str);
	return new_str;
}

inline char* copy_str(Allocator& allocator, const char* str)
{
	return copy_str(allocator, str, strlen32(str));
}

inline char* concat_str(Allocator& allocator, const char* str1, unsigned str1_len, const char* str2, unsigned str2_len)
{
	auto size = str1_len + str2_len + 1;
	auto new_str = (char*)allocator.alloc_raw(size);
	strcat(strcat(new_str, str1), str2);
	return new_str;
}

inline char* concat_str(Allocator& allocator, const char* str1, const char* str2)
{
	return concat_str(allocator, str1, strlen32(str1), str2, strlen32(str2));
}

}