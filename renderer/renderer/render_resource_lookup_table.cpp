#include "render_resource_lookup_table.h"
#include <cassert>
#include <cstring>

namespace bowtie
{

RenderResourceLookupTable::RenderResourceLookupTable()
{
	memset(&_lookup_table, 0, num_handles * sizeof(RenderResource));
}

void RenderResourceLookupTable::free(RenderResourceHandle handle)
{
	assert(handle < num_handles && "Handle is out of range");
	_lookup_table[handle.handle] = RenderResource();
}

const RenderResource& RenderResourceLookupTable::lookup(RenderResourceHandle handle) const
{
	assert(handle < num_handles && "Handle is out of range");
	auto& resource = _lookup_table[handle.handle];
	assert(resource.type != RenderResource::NotInitialized && "Trying to lookup unused render resource.");
	return resource;
}

void RenderResourceLookupTable::set(RenderResourceHandle handle, const RenderResource& resource)
{
	_lookup_table[handle.handle] = resource;
}

}
