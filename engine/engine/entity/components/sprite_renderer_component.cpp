#pragma once

#include "sprite_renderer_component.h"
#include "../../rect.h"
#include "../../material.h"
#include <base/vector4.h>
#include <base/matrix4.h>
#include <base/quad.h>
#include <cassert>

namespace bowtie
{

namespace
{

SpriteRendererComponentData initialize_data(void* buffer, uint32 size)
{
    SpriteRendererComponentData new_data;
    new_data.entity = (Entity*)buffer;
    new_data.color = (Color*)(new_data.entity + size);
    new_data.rect = (Rect*)(new_data.color + size);
    new_data.material = (Material*)(new_data.rect + size);
    new_data.render_handle = (RenderResourceHandle*)(new_data.material + size);
    new_data.geometry = (Quad*)(new_data.render_handle + size);
    new_data.depth = (int32*)(new_data.geometry + size);
    return new_data;
}

void copy_offset(SpriteRendererComponentData* from, SpriteRendererComponentData* to, uint32 num, uint32 from_offset, uint32 to_offset)
{
    memcpy(to->entity + to_offset, from->entity + from_offset, num * sizeof(Entity));
    memcpy(to->color + to_offset, from->color + from_offset, num * sizeof(Color));
    memcpy(to->rect + to_offset, from->rect + from_offset, num * sizeof(Rect));
    memcpy(to->material + to_offset, from->material + from_offset, num * sizeof(Material));
    memcpy(to->render_handle + to_offset, from->render_handle + from_offset, num * sizeof(RenderResourceHandle));
    memcpy(to->geometry + to_offset, from->geometry + from_offset, num * sizeof(Quad));
    memcpy(to->depth + to_offset, from->depth + from_offset, num * sizeof(int));
}

void copy(SpriteRendererComponentData* from, SpriteRendererComponentData* to, uint32 num)
{
    copy_offset(from, to, num, 0, 0);
}

void internal_copy(SpriteRendererComponentData* c, uint32 from, uint32 to)
{
    copy_offset(c, c, 1, from, to);
}

void swap(SpriteRendererComponent* c, uint32 i1, uint32 i2)
{
    hash::set(&c->header.map, c->data.entity[i1], i2);
    hash::set(&c->header.map, c->data.entity[i2], i1);
    internal_copy(&c->data, i2, c->header.num);
    internal_copy(&c->data, i1, i2);
    internal_copy(&c->data, c->header.num, i1);
}

void grow(SpriteRendererComponent* c, Allocator* allocator)
{
    const uint32 new_capacity = c->header.capacity == 0 ? 8 : c->header.capacity * 2;
    const uint32 bytes = new_capacity * sprite_renderer_component::component_size;
    void* buffer = allocator->alloc_raw(bytes);

    auto new_data = initialize_data(buffer, new_capacity);
    copy(&c->data, &new_data, c->header.num);
    c->data = new_data;

    allocator->dealloc(c->buffer);
    c->buffer = buffer;
    c->header.capacity = new_capacity;
}

void mark_dirty(SpriteRendererComponent* c, uint32 index)
{
    auto dd = component::mark_dirty(&c->header, index);

    if (dd.new_index == dd.old_index)
        return;

    swap(c, dd.old_index, dd.new_index);
}

}

namespace sprite_renderer_component
{

uint32 component_size = (sizeof(Entity) + sizeof(Color) + sizeof(Rect) + sizeof(Material) + sizeof(RenderResourceHandle) + sizeof(Quad) + sizeof(int));

void init(SpriteRendererComponent* c, Allocator* allocator)
{
    memset(c, 0, sizeof(SpriteRendererComponent));
    component::init(&c->header, allocator);
}

void deinit(SpriteRendererComponent* c, Allocator* allocator)
{
    component::deinit(&c->header);
    allocator->dealloc(c->buffer);
}

void create(SpriteRendererComponent* c, Entity e, Allocator* allocator, const Rect* rect, const Color* color)
{
    if (c->header.num >= c->header.capacity)
        grow(c, allocator);

    uint32 i = c->header.num++;
    hash::set(&c->header.map, e, i);
    c->data.entity[i] = e;
    c->data.color[i] = *color;
    c->data.rect[i] = *rect;
    c->data.material[i].render_handle = component::NotAssigned;
    c->data.render_handle[i] = component::NotAssigned;
    memset(c->data.geometry + i, 0, sizeof(Quad));
    c->data.depth[i] = 0;
    
    if (c->header.first_new == component::NotAssigned)
        c->header.first_new = i;
}

void destroy(SpriteRendererComponent* c, Entity e)
{
    uint32 i = hash::get(&c->header.map, e, 0u);
    hash::remove(&c->header.map, e);
    --c->header.num;

    if (i == c->header.num)
        return;
        
    internal_copy(&c->data, c->header.num, i);
}

void set_rect(SpriteRendererComponent* c, Entity e, const Rect* rect)
{
    auto i = hash::get(&c->header.map, e);
    c->data.rect[i] = *rect;
    mark_dirty(c, i);
}

const Rect* rect(SpriteRendererComponent* c, Entity e)
{
    return &c->data.rect[hash::get(&c->header.map, e)];
}

void set_color(SpriteRendererComponent* c, Entity e, const Color* color)
{
    auto i = hash::get(&c->header.map, e);
    c->data.color[i] = *color;
    mark_dirty(c, i);
}

const Color* color(SpriteRendererComponent* c, Entity e)
{
    return &c->data.color[hash::get(&c->header.map, e)];
}

void set_render_handle(SpriteRendererComponent* c, Entity e, RenderResourceHandle render_handle)
{
    c->data.render_handle[hash::get(&c->header.map, e)] = render_handle;
}

const Material* material(SpriteRendererComponent* c, Entity e)
{
    return &c->data.material[hash::get(&c->header.map, e)];
}

void set_material(SpriteRendererComponent* c, Entity e, Material* material)
{
    auto i = hash::get(&c->header.map, e);
    c->data.material[i] = *material;
    mark_dirty(c, i);
}

RenderResourceHandle render_handle(SpriteRendererComponent* c, Entity e)
{
    return c->data.render_handle[hash::get(&c->header.map, e)];
}

void set_geometry(SpriteRendererComponent* c, Entity e, const Quad* geometry)
{
    auto i = hash::get(&c->header.map, e);
    c->data.geometry[i] = *geometry;
    mark_dirty(c, i);
}

const Quad* transform(SpriteRendererComponent* c, Entity e)
{
    return &c->data.geometry[hash::get(&c->header.map, e)];
}

void set_depth(SpriteRendererComponent* c, Entity e, int32 depth)
{
    auto i = hash::get(&c->header.map, e);
    c->data.depth[i] = depth;
    mark_dirty(c, i);
}

void* copy_dirty_data(SpriteRendererComponent* c, Allocator* allocator)
{
    auto num_dirty = component::num_dirty(&c->header);
    void* buffer = allocator->alloc_raw(component_size * num_dirty);
    auto data = initialize_data(buffer, num_dirty);
    copy(&c->data, &data, num_dirty);
    return buffer;
}

void* copy_new_data(SpriteRendererComponent* c, Allocator* allocator)
{
    auto num_new = component::num_new(&c->header);
    void* buffer = allocator->alloc_raw(component_size * num_new);
    auto data = initialize_data(buffer, num_new);
    copy_offset(&c->data, &data, num_new, c->header.first_new, 0);
    return buffer;
}

SpriteRendererComponentData create_data_from_buffer(void* buffer, uint32 num)
{
    return initialize_data(buffer, num);
}

} // sprite_renderer_component

} // namespace bowtie
