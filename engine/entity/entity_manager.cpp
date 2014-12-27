#pragma once
#include "entity_manager.h"
#include <base/vector.h>
#include <base/memory.h>

namespace bowtie
{

namespace
{


uint32 get_next_index(EntityManager* m)
{
    if (m->index_holes.size == 0)
        return ++m->last_entity_index;

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

Entity create(EntityManager* m, World* world)
{
    uint32 index = get_next_index(m);

    if (index >= m->last_entity_index)
    {
        vector::resize(&m->generation, m->last_entity_index);
        m->generation[index] = 1;
    }
    else
        ++m->generation[index];

    Entity e = {};
    e.id = entity::create_id(index, m->generation[index]);
    e.world = world;
    return e;
}

void destroy(EntityManager* m, Entity entity)
{
    uint32 index = entity::index(entity);
    ++m->generation[index];
    
    if (index <= m->last_entity_index)
        vector::push(&m->index_holes, index);
}

bool is_alive(EntityManager* m, Entity entity)
{
    return m->generation[entity::index(entity)] == entity::generation(entity);
}

} // namespace entity_manager

} // namespace bowtie
