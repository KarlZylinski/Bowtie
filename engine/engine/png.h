#pragma once

namespace bowtie
{

struct Allocator;

struct UncompressedTexture
{
	void* data;
	unsigned data_size;
	unsigned width, height;
};

namespace png
{

UncompressedTexture load(const char* filename, Allocator* allocator);

}
}