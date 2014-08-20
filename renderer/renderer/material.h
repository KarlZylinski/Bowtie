#pragma once

#include <foundation/collection_types.h>
#include "render_resource_handle.h"
#include "uniform.h"

namespace bowtie
{

class Material
{
public:
	Material(Allocator& allocator, RenderResourceHandle shader);
	void add_uniform(const Uniform& uniform);
	const Array<Uniform>& uniforms() const;
	
private:
	RenderResourceHandle _shader;
	Array<Uniform> _uniforms;
};

}