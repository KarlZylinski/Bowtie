#include "shader_utils.h"
#include <base/file.h>
#include <base/memory.h>
#include <base/string_utils.h>

namespace bowtie
{
namespace shader_utils
{

SplitShader split_shader(const LoadedFile* shader_source)
{
    const char* delimiter = "#fragment";

    auto delimiter_len = strlen32(delimiter);
    auto shader_len = shader_source->size;

    uint32 matched_index = 0;
    uint32 fragment_start_index = 0;
    for (; fragment_start_index < shader_len; ++fragment_start_index)
    {
        if (shader_source->data[fragment_start_index] == delimiter[matched_index])
            ++matched_index;
        else
            matched_index = 0;

        if (matched_index == delimiter_len)
            break;
    }

    ++fragment_start_index;
    Assert(matched_index == delimiter_len && fragment_start_index <= shader_len, "Could not find fragment part of shader.");

    uint32 vertex_shader_source_len = fragment_start_index - delimiter_len;
    char* vertex_shader_source = (char*)temp_memory::alloc_raw(vertex_shader_source_len + 1);
    memcpy(vertex_shader_source, shader_source->data, vertex_shader_source_len);
    vertex_shader_source[vertex_shader_source_len] = 0;
    
    uint32 fragment_shader_source_len = shader_len - fragment_start_index;
    char* fragment_shader_source = (char*)temp_memory::alloc_raw(fragment_shader_source_len + 1);
    memcpy(fragment_shader_source, shader_source->data + fragment_start_index, fragment_shader_source_len);
    fragment_shader_source[fragment_shader_source_len] = 0;

    SplitShader ss;
    ss.fragment_source = fragment_shader_source;
    ss.vertex_source = vertex_shader_source;
    ss.fragment_source_len = fragment_shader_source_len + 1;
    ss.vertex_source_len = vertex_shader_source_len + 1;

    return ss;
}

}
}