#include "entity.h"

namespace bowtie
{

const uint32 entity_index_bits = 20;
const uint32 entity_index_mask = (1 << entity_index_bits) - 1;
const uint32 entity_generation_bits = 12;
const uint32 entity_generation_mask = (1 << entity_generation_bits) - 1;

namespace entity
{

uint32 index(Entity e)
{
    return e.id & entity_index_mask;
}

uint32 generation(Entity e)
{
    return (e.id >> entity_index_bits) & entity_generation_mask;
}

uint32 create_id(uint32 index, uint32 generation)
{
    return (generation << entity_index_bits) | index;
}

}

}