#pragma once

#include <engine/render_resource_handle.h>
#include <foundation/collection_types.h>
#include "uniform.h"
#include <stdint.h>

namespace bowtie
{

class RenderMaterial
{
public:
	RenderMaterial(Allocator& allocator, RenderResourceHandle shader);
	void add_uniform(const Uniform& uniform);
	void set_uniform_value(Allocator& allocator, uint64_t name, void* value);
	RenderResourceHandle shader() const;
	const Array<Uniform>& uniforms() const;
	
private:
	RenderResourceHandle _shader;
	Array<Uniform> _uniforms;
};

}