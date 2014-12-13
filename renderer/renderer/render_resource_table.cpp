#include "render_resource_table.h"
#include "render_resource.h"
#include <engine/render_resource_handle.h>
#include <cassert>

namespace bowtie
{

namespace render_resource_table
{

void free(RenderResource* table, RenderResourceHandle handle)
{
	assert(handle < render_resource_handle::num && "Handle is out of range");
	table[handle] = RenderResource();
}

RenderResource lookup(const RenderResource* table, RenderResourceHandle handle)
{
	assert(handle < render_resource_handle::num && "Handle is out of range");
	auto resource = table[handle];
	assert(resource.type != RenderResourceType::NotInitialized && "Trying to lookup unused render resource.");
	return resource;
}

void set(RenderResource* table, RenderResourceHandle handle, const RenderResource* resource)
{
	table[handle] = *resource;
}

} // namespace render_resource_lookup_table

} // namespace bowtie
