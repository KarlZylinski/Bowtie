#pragma once

#include <foundation/hash.h>

namespace bowtie
{

typedef uint32 Entity;

struct ComponentHeader
{
    // Maps entity id to component indices
    Hash<uint32> map;
    uint32 num;
    uint32 capacity;
    uint32 last_dirty_index;
    uint32 first_new;
};

namespace component
{
    void init(ComponentHeader* h, Allocator* allocator);
    void deinit(ComponentHeader* h);
    bool has_entity(const ComponentHeader* h, Entity e);
    uint32 num_dirty(const ComponentHeader* h);
    void reset_dirty(ComponentHeader* h);
    uint32 num_new(const ComponentHeader* h);
    void reset_new(ComponentHeader* h);
    
    struct DirtyData
    {
        uint32 old_index;
        uint32 new_index;
    };

    DirtyData mark_dirty(ComponentHeader* h, uint32 entity_index);
}

}
