#pragma once

namespace bowtie
{

struct Allocator;
struct LoadedFile;

struct SplitShader
{
	char* vertex_source;
	unsigned vertex_source_len;
	char* fragment_source;
	unsigned fragment_source_len;
};

namespace shader_utils
{

SplitShader split_shader(const LoadedFile& shader_source, Allocator& allocator);

}
}