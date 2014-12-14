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

Option<LoadedFile> load(const char* filename)
{
    FILE* fp;
    size_t filesize;
    uint8* data;
    
    char full_filename[512];
    strcpy(full_filename, resource_path());
    strcat(full_filename, filename);

    fp = fopen(full_filename, "rb");

    if (!fp)
        return option::none<LoadedFile>();

    fseek(fp, 0, SEEK_END);
    filesize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    data = (uint8*)temp_memory::alloc(uint32(filesize) + 1);

    if (!data)
        return option::none<LoadedFile>();

    fread(data, 1, filesize, fp);
    data[filesize] = 0;
    fclose(fp);

    LoadedFile lf;

    lf.data = data;
    lf.size = (uint32)filesize + 1;

    return option::some(lf);
}

}
}