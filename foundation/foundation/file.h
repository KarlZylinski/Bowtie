#pragma once

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

LoadedFile load(const char* filename, Allocator& allocator);

}
}