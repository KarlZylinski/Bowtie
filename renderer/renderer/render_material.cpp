#include "render_material.h"
#include <foundation/array.h>
#include "render_uniform_utils.h"

namespace bowtie
{

RenderMaterial::RenderMaterial(Allocator& allocator, RenderResourceHandle shader) : _shader(shader), _uniforms(allocator)
{
}

void RenderMaterial::add_uniform(const RenderUniform& uniform)
{
	array::push_back(_uniforms, uniform);
}

void RenderMaterial::set_uniform_value(Allocator& allocator, uint64_t name, void* value)
{
	for (unsigned i = 0; i < array::size(_uniforms); ++i)
	{
		auto& uniform = _uniforms[i];

		if (uniform.name == name)
		{
			render_uniform::set_value(uniform, allocator, value);
			break;
		}
	}
}

RenderResourceHandle RenderMaterial::shader() const
{
	return _shader;
}

const Array<RenderUniform>& RenderMaterial::uniforms() const
{
	return _uniforms;
}

}