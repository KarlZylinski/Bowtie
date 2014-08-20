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

const Array<Uniform>& Material::uniforms() const
{
	return _uniforms;
}

}