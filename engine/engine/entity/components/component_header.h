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
	unsigned first_new;
};

namespace component
{
	void init(ComponentHeader& h, Allocator& allocator);
	void deinit(ComponentHeader& h);
	bool has_entity(const ComponentHeader& h, Entity e);
	unsigned num_dirty(const ComponentHeader& h);
	void reset_dirty(ComponentHeader& h);
	unsigned num_new(const ComponentHeader& h);
	void reset_new(ComponentHeader& h);
	
	struct DirtyData
	{
		unsigned old_index;
		unsigned new_index;
	};

	DirtyData mark_dirty(ComponentHeader& h, unsigned entity_index);
}

}
