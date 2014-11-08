#pragma once

#include "component_header.h"

namespace bowtie
{

namespace component
{

void init(ComponentHeader& h, Allocator& allocator)
{
	memset(&h, 0, sizeof(ComponentHeader));
	h.map = hash::create<unsigned>(allocator);
	reset_dirty(h);
	reset_new(h);
}

void deinit(ComponentHeader& h)
{
	hash::deinit(h.map);
}

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

unsigned num_new(const ComponentHeader& h)
{
	if (h.first_new == (unsigned)-1)
		return 0;

	return h.num - h.first_new;
}

void reset_new(ComponentHeader& h)
{
	h.first_new = (unsigned)-1;
}

DirtyData mark_dirty(ComponentHeader& h, unsigned index)
{
	DirtyData dd = {
		index,
		index
	};

	if (h.last_dirty_index != (unsigned)-1 && index <= h.last_dirty_index)
		return dd;

	if (h.first_new != (unsigned)-1 && index >= h.first_new)
		return dd;

	dd.new_index = ++h.last_dirty_index;
	return dd;
}

} // namespace component_header

} // namespace bowtie