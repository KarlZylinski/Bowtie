#pragma once

namespace bowtie
{

class Allocator;

namespace file
{

struct LoadedFile
{
	unsigned char* data;
	unsigned size;
};

LoadedFile load(const char* filename, Allocator& allocator);

}
}