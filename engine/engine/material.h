#pragma once

#include <foundation/collection_types.h>
#include "resource_handle.h"

namespace bowtie
{

class Material
{
public:
	Material(ResourceHandle shader);
	
private:
	Array<Uniform> _uniforms;
	ResourceHandle _shader;
};

}