#pragma once

#include <base/hash.h>
#include "../entity.h"

namespace bowtie
{

struct ComponentHeader
{
    // Maps entity id to component indices
    uint32 index_by_entity_index[entity::max_entities];
    uint32 num;
    uint32 last_dirty_index;
    uint32 first_new;
};

namespace component
{
    const uint32 NotAssigned = (uint32)-1;
    void init(ComponentHeader* h);
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
