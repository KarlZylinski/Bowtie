#pragma once

#include <foundation/collection_types.h>
#include "render_resource.h"
#include "uniform.h"
#include <stdint.h>

namespace bowtie
{

class RenderMaterial
{
public:
	RenderMaterial(Allocator& allocator, RenderResource shader);
	void add_uniform(const Uniform& uniform);
	void set_uniform_value(uint64_t name, Vector4 value);
	RenderResource shader() const;
	const Array<Uniform>& uniforms() const;
	
private:
	RenderResource _shader;
	Array<Uniform> _uniforms;
};

}