#pragma once
#include "transform_component.h"
#include <base/vector2.h>
#include <base/matrix4.h>

#define GetIndex(c, e) c->header.index_by_entity_index[entity::index(e)]

namespace bowtie
{

namespace transform_component
{

TransformComponentData initialize_data(void* buffer, uint32 size)
{
    TransformComponentData new_data;
    new_data.entity = (Entity*)buffer;
    new_data.position = (Vector2*)(new_data.entity + size);
    new_data.rotation = (real32*)(new_data.position + size);
    new_data.pivot = (Vector2*)(new_data.rotation + size);
    new_data.parent_index = (uint32*)(new_data.pivot + size);
    new_data.first_child = (uint32*)(new_data.parent_index + size);
    new_data.next_sibling = (uint32*)(new_data.first_child + size);
    new_data.previous_sibling = (uint32*)(new_data.next_sibling + size);
    new_data.world_transform = (Matrix4*)(new_data.previous_sibling + size);
    return new_data;
}

void copy_offset(TransformComponentData* from, TransformComponentData* to, uint32 num, uint32 from_offset, uint32 to_offset)
{
    memcpy(to->entity + to_offset, from->entity + from_offset, num * sizeof(Entity));
    memcpy(to->position + to_offset, from->position + from_offset, num * sizeof(Vector2));
    memcpy(to->rotation + to_offset, from->rotation + from_offset, num * sizeof(real32));
    memcpy(to->pivot + to_offset, from->pivot + from_offset, num * sizeof(Vector2));
    memcpy(to->parent_index + to_offset, from->parent_index + from_offset, num * sizeof(uint32));
    memcpy(to->first_child + to_offset, from->first_child + from_offset, num * sizeof(uint32));
    memcpy(to->next_sibling + to_offset, from->next_sibling + from_offset, num * sizeof(uint32));
    memcpy(to->previous_sibling + to_offset, from->previous_sibling + from_offset, num * sizeof(uint32));
    memcpy(to->world_transform + to_offset, from->world_transform + from_offset, num * sizeof(Matrix4));
}

void copy(TransformComponentData* from, TransformComponentData* to, uint32 num)
{
    copy_offset(from, to, num, 0, 0);
}

void internal_copy(TransformComponentData* c, uint32 from, uint32 to)
{
    copy_offset(c, c, 1, from, to);
}

void set_parent_internal(TransformComponentData* d, uint32 index, uint32 parent_index)
{
    if (d->parent_index[index] == parent_index)
        return;

    // Remove any references to this transform from old parent and siblings.
    if (d->parent_index[index] != component::NotAssigned)
    {
        auto old_parent = d->parent_index[index];
        auto old_parent_child_iter = d->first_child[old_parent];

        while (old_parent_child_iter != component::NotAssigned)
        {
            if (old_parent_child_iter == index)
            {
                old_parent_child_iter = d->next_sibling[old_parent_child_iter];
                continue;
            }

            auto next = d->next_sibling[old_parent_child_iter];
            auto previous = d->previous_sibling[old_parent_child_iter];

            if (next != component::NotAssigned && previous != component::NotAssigned) // Removed from middle.
            {
                d->next_sibling[previous] = next;
                d->previous_sibling[next] = previous;
            }
            else if (next != component::NotAssigned) // Removed from start.
                d->previous_sibling[next] = component::NotAssigned;
            else if (previous != component::NotAssigned) // Removed from end.
                d->next_sibling[previous] = component::NotAssigned;

            break;
        }

        if (d->first_child[old_parent] == index)
            d->first_child[old_parent] = d->next_sibling[index];
    }
    
    // Unset any previous siblings to this component.
    d->previous_sibling[index] = component::NotAssigned;
    d->next_sibling[index] = component::NotAssigned;

    if (parent_index != component::NotAssigned)
    {
        // Parent already has children, insert at front of list.
        if (d->first_child[parent_index] != component::NotAssigned)
        {
            auto current_first_child = d->first_child[parent_index];
            d->previous_sibling[current_first_child] = index;
            d->next_sibling[index] = current_first_child;
        }

        d->first_child[parent_index] = index;
    }

    d->parent_index[index] = parent_index;
}

void update_child_parent_indices(TransformComponent* c, uint32 parent)
{
    auto child = c->data.first_child[parent];

    while (child != component::NotAssigned)
    {
        c->data.parent_index[child] = parent;
        child = c->data.next_sibling[child];
    }
}

void swap(TransformComponent* c, uint32 i1, uint32 i2)
{
    auto i1_parent = c->data.parent_index[i1];
    auto i2_parent = c->data.parent_index[i2];

    if (i1_parent != component::NotAssigned)
        set_parent_internal(&c->data, i1, component::NotAssigned);

    if (i2_parent != component::NotAssigned)
        set_parent_internal(&c->data, i2, component::NotAssigned);

    c->header.index_by_entity_index[entity::index(c->data.entity[i1])] = i2;
    c->header.index_by_entity_index[entity::index(c->data.entity[i2])] = i1;
    internal_copy(&c->data, i2, c->header.num);
    internal_copy(&c->data, i1, i2);
    internal_copy(&c->data, c->header.num, i1);

    if (i1_parent != component::NotAssigned)
        set_parent_internal(&c->data, i2, i1_parent);

    if (i2_parent != component::NotAssigned)
        set_parent_internal(&c->data, i1, i2_parent);

    update_child_parent_indices(c, i1);
    update_child_parent_indices(c, i2);
}

void mark_dirty(TransformComponent* c, uint32 index)
{
    auto dd = component::mark_dirty(&c->header, index);

    if (dd.new_index != dd.old_index)
        swap(c, dd.old_index, dd.new_index);

    auto child_iter = c->data.first_child[dd.new_index];

    // Mark all children dirty as well.
    while (child_iter != component::NotAssigned)
    {
        auto entity = c->data.entity[child_iter];
        mark_dirty(c, child_iter);
        child_iter = c->header.index_by_entity_index[entity::index(entity)]; // Index might change when swapping, refetch it.
        auto child_parent = c->data.parent_index[child_iter];
        Assert(c->data.parent_index[child_iter] != component::NotAssigned, "Swapped parent of transform child, but the parent is invalid after swap");

        if (child_iter < child_parent)
        {
            swap(c, child_iter, child_parent); // Parents must be before all children, otherwise updating will be wonky.
            child_iter = c->header.index_by_entity_index[entity::index(entity)];
        }

        child_iter = c->data.next_sibling[child_iter];
    }
}

uint32 component_size = sizeof(Entity) + sizeof(Vector2) + sizeof(real32) + sizeof(Vector2)
                            + sizeof(uint32) + sizeof(uint32) + sizeof(uint32) + sizeof(uint32)
                            + sizeof(Matrix4);

void init(TransformComponent* c)
{
    memset(c, 0, sizeof(TransformComponent));
    component::init(&c->header);
    const auto buffer_size = component_size * entity::max_entities;
    c->buffer = memory::alloc(&MainThreadMemory, buffer_size);
    c->data = initialize_data(c->buffer, entity::max_entities);
}

void create(TransformComponent* c, Entity e)
{
    auto i = c->header.num++;
    assert(i < entity::max_entities);
    c->header.index_by_entity_index[entity::index(e)] = i;
    c->data.entity[i] = e;
    c->data.position[i] = vector2::create(0, 0);
    c->data.rotation[i] = 0;
    c->data.pivot[i] = vector2::create(0, 0);
    c->data.parent_index[i] = component::NotAssigned;
    c->data.first_child[i] = component::NotAssigned;
    c->data.next_sibling[i] = component::NotAssigned;
    c->data.previous_sibling[i] = component::NotAssigned;
    c->data.world_transform[i] = matrix4::indentity();

    if (c->header.first_new == component::NotAssigned)
        c->header.first_new = i;
}

void destroy(TransformComponent* c, Entity e)
{
    auto i = GetIndex(c, e);
    --c->header.num;

    if (i == c->header.num)
        return;
        
    internal_copy(&c->data, c->header.num - 1, i);
}

void set_position(TransformComponent* c, Entity e, const Vector2* position)
{    
    auto i = GetIndex(c, e);
    c->data.position[i] = *position;
    mark_dirty(c, i);
}

const Vector2* position(TransformComponent* c, Entity e)
{
    return &c->data.position[GetIndex(c, e)];
}

void set_rotation(TransformComponent* c, Entity e, real32 rotation)
{
    auto i = GetIndex(c, e);
    c->data.rotation[i] = rotation;
    mark_dirty(c, i);
}

real32 rotation(TransformComponent* c, Entity e)
{
    return c->data.rotation[GetIndex(c, e)];
}

void set_pivot(TransformComponent* c, Entity e, const Vector2* pivot)
{
    auto i = GetIndex(c, e);
    c->data.pivot[i] = *pivot;
    mark_dirty(c, i);
}

const Vector2* pivot(TransformComponent* c, Entity e)
{
    return &c->data.pivot[GetIndex(c, e)];
}

void set_parent(TransformComponent* c, Entity e, Entity parent_entity)
{
    auto i = GetIndex(c, e);
    set_parent_internal(&c->data, i, GetIndex(c, parent_entity));
    mark_dirty(c, i);
}

Entity parent(TransformComponent* c, Entity e)
{
    return c->data.entity[c->data.parent_index[GetIndex(c, e)]];
}

void set_world_transform(TransformComponent* c, Entity e, const Matrix4* world_transform)
{
    auto i = GetIndex(c, e);
    c->data.world_transform[i] = *world_transform;
    mark_dirty(c, i);
}

const Matrix4* world_transform(TransformComponent* c, Entity e)
{
    return &c->data.world_transform[GetIndex(c, e)];
}

void* copy_dirty_data(TransformComponent* c)
{
    auto num_dirty = c->header.last_dirty_index + 1;
    void* buffer = temp_memory::alloc_raw(component_size * num_dirty);
    auto data = initialize_data(buffer, num_dirty);
    copy(&c->data, &data, num_dirty);
    return buffer;
}

} // namespace transform_component

} // namespace bowtie
