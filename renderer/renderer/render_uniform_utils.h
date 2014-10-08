#pragma once

namespace bowtie
{

struct Vector4;
class Allocator;
struct RenderUniform;

namespace render_uniform
{
	void set_value(RenderUniform& uniform, Allocator& allocator, void* value);
};

}