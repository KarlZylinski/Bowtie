#pragma once

namespace bowtie
{

class Allocator;
namespace file { struct LoadedFile; }

namespace shader_utils
{

struct SplitShader
{
	char* vertex_source;
	unsigned vertex_source_len;
	char* fragment_source;
	unsigned fragment_source_len;
};

SplitShader split_shader(const file::LoadedFile& shader_source, Allocator& allocator);

}
}