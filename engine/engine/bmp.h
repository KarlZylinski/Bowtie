#pragma once

namespace bowtie
{

class Allocator;

struct BmpTexture
{
	void* data;
	unsigned data_size;
	unsigned width, height;
};

namespace bmp
{

BmpTexture load(const char* filename, Allocator& allocator);

}
}