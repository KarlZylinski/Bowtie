#pragma once
#include "sprite_renderer_component.h"
#include "../../rect.h"
#include "../../material.h"
#include <base/vector4.h>
#include <base/matrix4.h>
#include <base/quad.h>

#define GetIndex(c, e) c->header.index_by_entity_index[entity::index(e)]

namespace bowtie
{

namespace sprite_renderer_component
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
    memcpy(to->depth + to_offset, from->depth + from_offset, num * sizeof(int32));
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
    c->header.index_by_entity_index[entity::index(c->data.entity[i1])] = i2;
    c->header.index_by_entity_index[entity::index(c->data.entity[i2])] = i1;
    internal_copy(&c->data, i2, c->header.num);
    internal_copy(&c->data, i1, i2);
    internal_copy(&c->data, c->header.num, i1);
}

void mark_dirty(SpriteRendererComponent* c, uint32 index)
{
    auto dd = component::mark_dirty(&c->header, index);

    if (dd.new_index == dd.old_index)
        return;

    swap(c, dd.old_index, dd.new_index);
}

uint32 component_size = (sizeof(Entity) + sizeof(Color) + sizeof(Rect) + sizeof(Material) + sizeof(RenderResourceHandle) + sizeof(Quad) + sizeof(int32));

void init(SpriteRendererComponent* c)
{
    memset(c, 0, sizeof(SpriteRendererComponent));
    component::init(&c->header);
    const auto buffer_size = component_size * entity::max_entities;
    c->buffer = memory::alloc(&MainThreadMemory, buffer_size);
    c->data = initialize_data(c->buffer, entity::max_entities);
}

void create(Entity e, const Rect* rect, const Color* color)
{
    auto c = &e.world->sprite_renderer_components;
    auto i = c->header.num++;
    assert(i < entity::max_entities);
    c->header.index_by_entity_index[entity::index(e)] = i;
    c->data.entity[i] = e;
    c->data.color[i] = *color;
    c->data.rect[i] = *rect;
    c->data.material[i].render_handle = component::NotAssigned;
    c->data.render_handle[i] = NotInitialized;
    memset(c->data.geometry + i, 0, sizeof(Quad));
    c->data.depth[i] = 0;
    
    if (c->header.first_new == component::NotAssigned)
        c->header.first_new = i;
}

void destroy(SpriteRendererComponent* c, Entity e)
{
    auto i = GetIndex(c, e);
    --c->header.num;

    if (i == c->header.num)
        return;
        
    internal_copy(&c->data, c->header.num, i);
}

void set_rect(SpriteRendererComponent* c, Entity e, const Rect* rect)
{
    auto i = GetIndex(c, e);
    c->data.rect[i] = *rect;
    mark_dirty(c, i);
}

const Rect* rect(SpriteRendererComponent* c, Entity e)
{
    return &c->data.rect[GetIndex(c, e)];
}

void set_color(SpriteRendererComponent* c, Entity e, const Color* color)
{
    auto i = GetIndex(c, e);
    c->data.color[i] = *color;
    mark_dirty(c, i);
}

const Color* color(SpriteRendererComponent* c, Entity e)
{
    return &c->data.color[GetIndex(c, e)];
}

void set_render_handle(SpriteRendererComponent* c, Entity e, RenderResourceHandle render_handle)
{
    c->data.render_handle[GetIndex(c, e)] = render_handle;
}

const Material* material(SpriteRendererComponent* c, Entity e)
{
    return &c->data.material[GetIndex(c, e)];
}

void set_material(Entity e, Material* material)
{
    auto c = &e.world->sprite_renderer_components;
    auto i = GetIndex(c, e);
    c->data.material[i] = *material;
    mark_dirty(c, i);
}

RenderResourceHandle render_handle(SpriteRendererComponent* c, Entity e)
{
    return c->data.render_handle[GetIndex(c, e)];
}

void set_geometry(SpriteRendererComponent* c, Entity e, const Quad* geometry)
{
    auto i = GetIndex(c, e);
    c->data.geometry[i] = *geometry;
    mark_dirty(c, i);
}

const Quad* transform(SpriteRendererComponent* c, Entity e)
{
    return &c->data.geometry[GetIndex(c, e)];
}

void set_depth(SpriteRendererComponent* c, Entity e, int32 depth)
{
    auto i = GetIndex(c, e);
    c->data.depth[i] = depth;
    mark_dirty(c, i);
}

void* copy_dirty_data(SpriteRendererComponent* c)
{
    auto num_dirty = component::num_dirty(&c->header);
    void* buffer = temp_memory::alloc_raw(component_size * num_dirty);
    auto data = initialize_data(buffer, num_dirty);
    copy(&c->data, &data, num_dirty);
    return buffer;
}

void* copy_new_data(SpriteRendererComponent* c)
{
    auto num_new = component::num_new(&c->header);
    void* buffer = temp_memory::alloc_raw(component_size * num_new);
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
