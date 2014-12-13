#pragma once

namespace bowtie
{

struct Allocator;
struct LoadedFile;

struct SplitShader
{
	char* vertex_source;
	uint32 vertex_source_len;
	char* fragment_source;
	uint32 fragment_source_len;
};

namespace shader_utils
{

SplitShader split_shader(const LoadedFile* shader_source, Allocator* allocator);

}
}