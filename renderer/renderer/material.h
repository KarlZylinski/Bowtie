#pragma once

#include <foundation/collection_types.h>
#include "render_resource_handle.h"
#include "uniform.h"
#include <stdint.h>

namespace bowtie
{

class Material
{
public:
	Material(Allocator& allocator, RenderResourceHandle shader);
	void add_uniform(const Uniform& uniform);
	void set_uniform_value(uint64_t name, Vector4 value);
	RenderResourceHandle shader() const;
	const Array<Uniform>& uniforms() const;
	
private:
	RenderResourceHandle _shader;
	Array<Uniform> _uniforms;
};

}