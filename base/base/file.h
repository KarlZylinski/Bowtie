#pragma once
#include "option.h"

namespace bowtie
{

struct Allocator;

struct LoadedFile
{
    uint8* data;
    uint32 size;
};

namespace file
{

Option<LoadedFile> load(const char* filename, Allocator* allocator);

}
}