#include "render_material.h"
#include <foundation/array.h>
#include "uniform_utils.h"

namespace bowtie
{

RenderMaterial::RenderMaterial(Allocator& allocator, RenderResourceHandle shader) : _shader(shader), _uniforms(allocator)
{
}

void RenderMaterial::add_uniform(const Uniform& uniform)
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
			switch (uniform.type)
			{
			case Uniform::Float:
				uniform::SetValue(uniform, allocator, *(float*)value);
				break;
			}
			break;
		}
	}
}

RenderResourceHandle RenderMaterial::shader() const
{
	return _shader;
}

const Array<Uniform>& RenderMaterial::uniforms() const
{
	return _uniforms;
}

}