#pragma once

#include <engine/render_resource_handle.h>
#include "render_resource.h"

namespace bowtie
{

class RenderResourceLookupTable
{
public:
	RenderResourceLookupTable();

	void free(RenderResourceHandle handle);
	const RenderResource& lookup(RenderResourceHandle handle) const;
	void set(RenderResourceHandle handle, const RenderResource& resource);
	
	static const unsigned num_handles = 4000;

private:
	RenderResource _lookup_table[num_handles];
};


} // namespace bowtie;