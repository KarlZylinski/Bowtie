#include "render_resource_table.h"
#include "render_resource.h"
#include <cassert>

namespace bowtie
{

namespace render_resource_table
{

void free(RenderResource* table, RenderResourceHandle handle)
{
	assert(handle < size && "Handle is out of range");
	table[handle.handle] = RenderResource();
}

const RenderResource& lookup(const RenderResource* table, RenderResourceHandle handle)
{
	assert(handle < size && "Handle is out of range");
	auto& resource = table[handle.handle];
	assert(resource.type != RenderResource::NotInitialized && "Trying to lookup unused render resource.");
	return resource;
}

void set(RenderResource* table, RenderResourceHandle handle, const RenderResource& resource)
{
	table[handle.handle] = resource;
}

} // namespace render_resource_lookup_table

} // namespace bowtie
