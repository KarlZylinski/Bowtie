#pragma once

#include <foundation/hash.h>

namespace bowtie
{

typedef unsigned Entity;

struct ComponentHeader
{
	// Maps entity id to rectangle renderer components
	Hash<unsigned> map;
	unsigned num;
	unsigned capacity;
	unsigned last_dirty_index;
};

namespace component
{
	bool has_entity(const ComponentHeader& h, Entity e);
}

}
