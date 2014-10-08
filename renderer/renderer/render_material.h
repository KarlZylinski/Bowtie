#pragma once

#include <engine/render_resource_handle.h>
#include <foundation/collection_types.h>
#include "render_uniform.h"
#include <stdint.h>

namespace bowtie
{

class RenderMaterial
{
public:
	RenderMaterial(Allocator& allocator, RenderResourceHandle shader);
	void add_uniform(const RenderUniform& uniform);
	void set_uniform_value(Allocator& allocator, uint64_t name, void* value);
	RenderResourceHandle shader() const;
	const Array<RenderUniform>& uniforms() const;
	
private:
	RenderResourceHandle _shader;
	Array<RenderUniform> _uniforms;
};

}