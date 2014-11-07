#pragma once

#include "entity_manager.h"
#include <foundation/array.h>
#include <foundation/memory.h>
#include <cassert>

namespace bowtie
{

namespace
{

const unsigned entity_index_bits = 20;
const unsigned entity_index_mask = (1 << entity_index_bits) - 1;
const unsigned entity_generation_bits = 12;
const unsigned entity_generation_mask = (1 << entity_generation_bits) - 1;

unsigned entity_index(Entity e)
{
	return e & entity_index_mask;
}

unsigned entity_generation(Entity e)
{
	return (e >> entity_index_bits) & entity_generation_mask;
}

unsigned get_next_index(EntityManager& manager)
{
	if (array::size(manager.index_holes) == 0) {
		unsigned new_index = ++manager.last_entity_index;;
		assert(new_index < (1 << entity_generation_bits));
		return new_index; 
	}

	unsigned hole_index = rand() % array::size(manager.index_holes);
	unsigned index = manager.index_holes[hole_index];
	array::remove_at(manager.index_holes, hole_index);
	return index;
}

} // anonymous namespace

namespace entity_manager
{

void init(EntityManager& manager, Allocator& allocator)
{
	manager.last_entity_index = 0;
	array::init(manager.index_holes, allocator);
	array::init(manager.generation, allocator);
}

void deinit(EntityManager& manager)
{
	array::deinit(manager.index_holes);
	array::deinit(manager.generation);
}

Entity create(EntityManager& manager)
{
	unsigned index = get_next_index(manager);

	if (index >= manager.last_entity_index)
	{
		array::resize(manager.generation, manager.last_entity_index);
		manager.generation[index] = 1;
	}
	else
		++manager.generation[index];

	return (manager.generation[index] << entity_index_bits) | index;
}

void destroy(EntityManager& manager, Entity entity)
{
	unsigned index = entity_index(entity);
	++manager.generation[index];
	
	if (index <= manager.last_entity_index)
		array::push_back(manager.index_holes, index);
}

bool is_alive(EntityManager& manager, Entity entity)
{
	return manager.generation[entity_index(entity)] == entity_generation(entity);
}

} // namespace entity_manager

} // namespace bowtie
