#pragma once

#include "entity_manager.h"
#include <base/vector.h>
#include <base/memory.h>
#include <cassert>

namespace bowtie
{

namespace
{

const uint32 entity_index_bits = 20;
const uint32 entity_index_mask = (1 << entity_index_bits) - 1;
const uint32 entity_generation_bits = 12;
const uint32 entity_generation_mask = (1 << entity_generation_bits) - 1;

uint32 entity_index(Entity e)
{
    return e & entity_index_mask;
}

uint32 entity_generation(Entity e)
{
    return (e >> entity_index_bits) & entity_generation_mask;
}

uint32 get_next_index(EntityManager* m)
{
    if (m->index_holes.size == 0) {
        uint32 new_index = ++m->last_entity_index;;
        assert(new_index < (1 << entity_generation_bits));
        return new_index; 
    }

    uint32 hole_index = rand() % m->index_holes.size;
    uint32 index = m->index_holes[hole_index];
    vector::remove_at(&m->index_holes, hole_index);
    return index;
}

} // anonymous namespace

namespace entity_manager
{

void init(EntityManager* m, Allocator* allocator)
{
    m->last_entity_index = 0;
    vector::init(&m->index_holes, allocator);
    vector::init(&m->generation, allocator);
}

void deinit(EntityManager* m)
{
    vector::deinit(&m->index_holes);
    vector::deinit(&m->generation);
}

Entity create(EntityManager* m)
{
    uint32 index = get_next_index(m);

    if (index >= m->last_entity_index)
    {
        vector::resize(&m->generation, m->last_entity_index);
        m->generation[index] = 1;
    }
    else
        ++m->generation[index];

    return (m->generation[index] << entity_index_bits) | index;
}

void destroy(EntityManager* m, Entity entity)
{
    uint32 index = entity_index(entity);
    ++m->generation[index];
    
    if (index <= m->last_entity_index)
        vector::push(&m->index_holes, index);
}

bool is_alive(EntityManager* m, Entity entity)
{
    return m->generation[entity_index(entity)] == entity_generation(entity);
}

} // namespace entity_manager

} // namespace bowtie
