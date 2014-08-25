#include "material.h"
#include <foundation/array.h>

namespace bowtie
{

Material::Material(Allocator& allocator, RenderResourceHandle shader) : _shader(shader), _uniforms(allocator)
{
}

void Material::add_uniform(const Uniform& uniform)
{
	array::push_back(_uniforms, uniform);
}

void Material::set_uniform_value(uint64_t name, Vector4 value)
{
	for (unsigned i = 0; i < array::size(_uniforms); ++i)
	{
		auto& uniform = _uniforms[i];

		if (uniform.name == name)
		{
			uniform.value = value;
			break;
		}
	}
}

RenderResourceHandle Material::shader() const
{
	return _shader;
}

const Array<Uniform>& Material::uniforms() const
{
	return _uniforms;
}

}