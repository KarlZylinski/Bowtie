#pragma once

#include "component_header.h"

namespace bowtie
{

namespace component
{

bool has_entity(const ComponentHeader& h, Entity e)
{
	return hash::has(h.map, e);
}

unsigned num_dirty(const ComponentHeader& h)
{
	return h.last_dirty_index + 1;
}

void reset_dirty(ComponentHeader& h)
{
	h.last_dirty_index = (unsigned)-1;
}

} // namespace component_header

} // namespace bowtie
