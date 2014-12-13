#pragma once

namespace bowtie
{

struct Allocator;

struct UncompressedTexture
{
	void* data;
	uint32 data_size;
	uint32 width, height;
};

namespace png
{

UncompressedTexture load(const char* filename, Allocator* allocator);

}
}