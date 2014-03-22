#include "file.h"
#include "memory.h"
#include "resource_path.h"
#include <stdio.h>
#include <string.h>
#include <cassert>

namespace bowtie
{
namespace file
{

char* load(const char* filename, Allocator& allocator)
{
	FILE * fp;
	size_t filesize;
	char * data;
	
	char full_filename[512];
	strcpy(full_filename, resource_path());
	strcat(full_filename, filename);

	fp = fopen(full_filename, "rb");

	assert(fp);

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	data = (char*)allocator.allocate(unsigned(filesize) + 1);
	assert(data);

	fread(data, 1, filesize, fp);
	data[filesize] = 0;
	fclose(fp);

	return data;
}

}
}