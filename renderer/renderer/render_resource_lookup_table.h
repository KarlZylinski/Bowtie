#pragma once

#include <engine/resource_handle.h>
#include "render_resource.h"

namespace bowtie
{

class RenderResourceLookupTable
{
public:
	RenderResourceLookupTable();

	void free(ResourceHandle key);
	RenderResource lookup(ResourceHandle handle) const;
	void set(ResourceHandle key, RenderResource value);
	
	static const unsigned num_handles = 4000;

private:
	RenderResource _lookup_table[num_handles];
};


} // namespace bowtie;