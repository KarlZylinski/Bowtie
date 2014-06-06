#pragma once

namespace bowtie
{

class Allocator;

struct UncompressedTexture
{
	void* data;
	unsigned data_size;
	unsigned width, height;
};

namespace png
{

UncompressedTexture load(const char* filename, Allocator& allocator);

}
}