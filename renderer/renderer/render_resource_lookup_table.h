#pragma once

#include <engine/resource_handle.h>
#include "render_resource_handle.h"

namespace bowtie
{

class RenderResourceLookupTable
{
public:
	RenderResourceLookupTable();

	void free(ResourceHandle key);
	RenderResourceHandle lookup(ResourceHandle handle) const;
	void set(ResourceHandle key, RenderResourceHandle value);
	
	static const unsigned num_handles = 4000;

private:
	RenderResourceHandle _lookup_table[num_handles];
};


} // namespace bowtie;