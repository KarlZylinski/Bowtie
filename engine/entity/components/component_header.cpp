#pragma once

#include "component_header.h"

namespace bowtie
{

namespace component
{

void init(ComponentHeader* h)
{
    memset(h, 0, sizeof(ComponentHeader));
    reset_dirty(h);
    reset_new(h);
}

bool has_entity(const ComponentHeader* h, Entity e)
{
    return entity::index(e) <= h->num;
}

uint32 num_dirty(const ComponentHeader* h)
{
    return h->last_dirty_index + 1;
}

void reset_dirty(ComponentHeader* h)
{
    h->last_dirty_index = NotAssigned;
}

uint32 num_new(const ComponentHeader* h)
{
    if (h->first_new == NotAssigned)
        return 0;

    return h->num - h->first_new;
}

void reset_new(ComponentHeader* h)
{
    h->first_new = NotAssigned;
}

DirtyData mark_dirty(ComponentHeader* h, uint32 index)
{
    DirtyData dd = {
        index,
        index
    };

    if (h->last_dirty_index != NotAssigned && index <= h->last_dirty_index)
        return dd;

    if (h->first_new != NotAssigned && index >= h->first_new)
        return dd;

    dd.new_index = ++h->last_dirty_index;
    return dd;
}

} // namespace component_header

} // namespace bowtie
