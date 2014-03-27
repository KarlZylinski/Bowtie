#include "bmp.h"

#include "resource_path.h"

#include <cassert>
#include <cstring>
#include <cstdio>
#include <foundation/memory.h>

namespace bowtie
{
namespace bmp
{

BmpTexture load(const char* filename, Allocator& allocator)
{
	unsigned char header[54];
	unsigned data_pos;
	unsigned width, height;
	unsigned image_size;

	unsigned char* data;

	char full_filename[512];
	strcpy(full_filename, resource_path());
	strcat(full_filename, filename);

	FILE* file = fopen(full_filename, "rb");
 
	if(!file || fread(header, 1, 54, file) != 54 || !file || header[0] != 'B' || header[1] != 'M')
		assert(!"Not a BMP texture");

	data_pos = *(int*)&(header[0x0A]);
	image_size = *(int*)&(header[0x22]);
	width = *(int*)&(header[0x12]);
	height = *(int*)&(header[0x16]);

	if(image_size == 0) image_size = width * height * 3;
	if(data_pos == 0) data_pos = 54;

	data = (unsigned char*)allocator.allocate(image_size);

	fread(data, 1, image_size, file);
	fclose(file);

	BmpTexture bmp;
	bmp.data = data;
	bmp.data_size = image_size;
	bmp.width = width;
	bmp.height = height;

	return bmp;
}

}
}