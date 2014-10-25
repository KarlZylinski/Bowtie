#pragma once

#include "option.h"

namespace bowtie
{

class Allocator;

struct LoadedFile
{
	unsigned char* data;
	unsigned size;
};

namespace file
{

Option<LoadedFile> load(const char* filename, Allocator& allocator);

}
}