#pragma once

#include <cstring>
#include "vector.h"

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

inline uint32 strlen32(const char* str)
{
    if (str == 0)
        return 0;

    return uint32(strlen(str));
}

inline Vector<char*> split(const char* str, char delimiter)
{
    Vector<char*> words;
    vector::init(&words);
    auto len_str = strlen(str);

    if (len_str == 0)
        return words;

    auto len = len_str + 1;
    uint32 current_word_len = 0;

    for (uint32 i = 0; i < len; ++i)
    {
        if (str[i] == delimiter || i == len - 1)
        {
            char* word = (char*)temp_memory::alloc_raw(current_word_len + 1);
            memcpy(word, str + i - current_word_len, current_word_len);
            word[current_word_len] = 0;
            vector::push(&words, word);
            current_word_len = 0;
        }
        else
            ++current_word_len;
    }

    return words;
}

inline real32 real32_from_str(const char* str)
{
    return (real32)strtod(str, nullptr);
}

inline uint32 uint32_from_str(const char* str)
{
    return (uint32)strtoul(str, nullptr, 10);
}

inline char* copy_str(const char* str, uint32 len)
{
    auto size = len + 1;
    auto new_str = (char*)temp_memory::alloc_raw(size);
    strcpy_s(new_str, size, str);
    return new_str;
}

inline char* copy_str(const char* str)
{
    return copy_str(str, strlen32(str));
}

inline char* concat_str(const char* str1, uint32 str1_len, const char* str2, uint32 str2_len)
{
    auto size = str1_len + str2_len + 1;
    auto new_str = (char*)temp_memory::alloc_raw(size);
    strcat(strcat(new_str, str1), str2);
    return new_str;
}

inline char* concat_str(const char* str1, const char* str2)
{
    return concat_str(str1, strlen32(str1), str2, strlen32(str2));
}

}