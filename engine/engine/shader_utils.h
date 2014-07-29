#pragma once

namespace bowtie
{

class Allocator;
struct LoadedFile;

namespace shader_utils
{

struct SplitShader
{
	char* vertex_source;
	unsigned vertex_source_len;
	char* fragment_source;
	unsigned fragment_source_len;
};

SplitShader split_shader(const LoadedFile& shader_source, Allocator& allocator);

}
}