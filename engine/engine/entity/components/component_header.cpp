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

} // namespace component_header

} // namespace bowtie
