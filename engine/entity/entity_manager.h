#pragma once

#include "entity.h"
#include <base/collection_types.h>

namespace bowtie
{

struct Allocator;

struct EntityManager
{
    uint32 last_entity_index;
    // All free entity indices in the range (0, last_entity_id_index)
    Vector<uint32> index_holes;
    Vector<uint32> generation;
};

namespace entity_manager
{
    void init(EntityManager* manager, Allocator* allocator);
    void deinit(EntityManager* manager);
    Entity create(EntityManager* manager, World* world);
    void destroy(EntityManager* manager, Entity entity);
    bool is_alive(EntityManager* manager, Entity entity);
}

}