#include "render_resource_lookup_table.h"
#include <cassert>
#include <cstring>

namespace bowtie
{

RenderResourceLookupTable::RenderResourceLookupTable()
{
	memset(&_lookup_table, 0, num_handles * sizeof(RenderResourceHandle));
}

void RenderResourceLookupTable::free(ResourceHandle key)
{
	_lookup_table[key.handle] = RenderResourceHandle();
}

RenderResourceHandle RenderResourceLookupTable::lookup(ResourceHandle handle) const
{
	assert(handle.type == ResourceHandle::Handle && "Resource is not of handle-type");
	assert(handle.handle < num_handles && "Handle is out of range");
	return _lookup_table[handle.handle];
}

void RenderResourceLookupTable::set(ResourceHandle key, RenderResourceHandle value)
{
	_lookup_table[key.handle] = value;
}

}
