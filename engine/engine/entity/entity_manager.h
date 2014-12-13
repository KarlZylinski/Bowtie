#pragma once

#include "entity.h"
#include <foundation/collection_types.h>

namespace bowtie
{

struct Allocator;

struct EntityManager
{
	unsigned last_entity_index;
	// All free entity indices in the range (0, last_entity_id_index)
	Array<unsigned> index_holes;
	Array<unsigned> generation;
};

namespace entity_manager
{
	void init(EntityManager* manager, Allocator* allocator);
	void deinit(EntityManager* manager);
	Entity create(EntityManager* manager);
	void destroy(EntityManager* manager, Entity entity);
	bool is_alive(EntityManager* manager, Entity entity);
}

}