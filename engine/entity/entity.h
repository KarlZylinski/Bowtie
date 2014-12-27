#pragma once

namespace bowtie
{

struct World;

typedef uint32 EntityId;

struct Entity
{
    EntityId id;
    World* world;
};

namespace entity
{
    static const uint32 max_entities = 16000;
    uint32 index(Entity e);
    uint32 generation(Entity e);
    uint32 create_id(uint32 index, uint32 generation);
}

}