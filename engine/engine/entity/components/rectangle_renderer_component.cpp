#pragma once

#include "rectangle_renderer_component.h"
#include "../../rect.h"
#include "../../material.h"
#include <foundation/vector4.h>
#include <foundation/matrix4.h>
#include <foundation/quad.h>
#include <cassert>

namespace bowtie
{

namespace
{

RectangleRendererComponentData initialize_data(void* buffer, unsigned size)
{
	RectangleRendererComponentData new_data;
	new_data.color = (Color*)buffer;
	new_data.rect = (Rect*)(new_data.color + size);
	new_data.material = (RenderResourceHandle*)(new_data.rect + size);
	new_data.render_handle = (RenderResourceHandle*)(new_data.material + size);
	new_data.geometry = (Quad*)(new_data.render_handle + size);
	return new_data;
}

void move_one(RectangleRendererComponent& c, unsigned from, unsigned to)
{
	c.data.color[to] = c.data.color[from];
	c.data.rect[to] = c.data.rect[from];
	c.data.material[to] = c.data.material[from];
	c.data.render_handle[to] = c.data.render_handle[from];
	c.data.geometry[to] = c.data.geometry[from];
}

void copy(RectangleRendererComponent& c, RectangleRendererComponentData& dest, unsigned num)
{
	memcpy(dest.color, c.data.color, num * sizeof(Color));
	memcpy(dest.rect, c.data.rect, num * sizeof(Rect));
	memcpy(dest.material, c.data.material, num * sizeof(Material));
	memcpy(dest.render_handle, c.data.render_handle, num * sizeof(RenderResourceHandle));
	memcpy(dest.geometry, c.data.geometry, num * sizeof(Quad));
}

void copy(RectangleRendererComponent& c, RectangleRendererComponentData& dest)
{
	copy(c, dest, c.header.num);
}

void grow(RectangleRendererComponent& c, Allocator& allocator)
{
	const unsigned new_capacity = c.header.capacity == 0 ? 8 : c.header.capacity * 2;
	const unsigned bytes = new_capacity * rectangle_renderer_component::component_size;
	void* buffer = allocator.allocate(bytes);

	auto new_data = initialize_data(buffer, new_capacity);
	copy(c, new_data);
	c.data = new_data;

	allocator.deallocate(c.buffer);
	c.buffer = buffer;
	c.header.capacity = new_capacity;
}

void mark_dirty(RectangleRendererComponent& c, Entity e)
{
	auto entity_index = hash::get(c.header.map, e, 0u);

	if (c.header.last_dirty_index != (unsigned)-1 && entity_index <= c.header.last_dirty_index)
		return;

	auto current_dirty_index = ++c.header.last_dirty_index;

	if (current_dirty_index == entity_index)
		return;

	auto color_at_index = c.data.color[current_dirty_index];
	auto rect_at_index = c.data.rect[current_dirty_index];
	auto material_at_index = c.data.material[current_dirty_index];
	auto render_handle_at_index = c.data.render_handle[current_dirty_index];
	auto transform_at_index = c.data.geometry[current_dirty_index];
	move_one(c, entity_index, current_dirty_index);
	c.data.color[entity_index] = color_at_index;
	c.data.rect[entity_index] = rect_at_index;
	c.data.material[entity_index] = material_at_index;
	c.data.render_handle[entity_index] = render_handle_at_index;
	c.data.geometry[entity_index] = transform_at_index;
}

}

namespace rectangle_renderer_component
{

unsigned component_size = (sizeof(Color) + sizeof(Rect) + sizeof(RenderResourceHandle) + sizeof(RenderResourceHandle) + sizeof(Quad));

void init(RectangleRendererComponent& c, Allocator& allocator)
{
	memset(&c, 0, sizeof(RectangleRendererComponent));
	c.header.map = hash::create<unsigned>(allocator);
	c.header.last_dirty_index = (unsigned)-1;
}

void deinit(RectangleRendererComponent& c, Allocator& allocator)
{
	hash::deinit(c.header.map);
	allocator.deallocate(c.buffer);
}

void create(RectangleRendererComponent& c, Entity e, Allocator& allocator, const Rect& rect, const Color& color)
{
	if (c.header.num >= c.header.capacity)
		grow(c, allocator);

	unsigned i = c.header.num++;
	hash::set(c.header.map, e, i);
	c.data.color[i] = color;
	c.data.rect[i] = rect;
	c.data.material[i] = RenderResourceHandle::NotInitialized;
	c.data.render_handle[i] = RenderResourceHandle::NotInitialized;
	memset(c.data.geometry + i, 0, sizeof(Quad));
}

void destroy(RectangleRendererComponent& c, Entity e)
{
	unsigned i = hash::get(c.header.map, e, 0u);
	hash::remove(c.header.map, e);
	--c.header.num;

	if (i == c.header.num)
		return;
		
	move_one(c, c.header.num, i);
}

void set_rect(RectangleRendererComponent& c, Entity e, const Rect& rect)
{	
	c.data.rect[hash::get(c.header.map, e)] = rect;
	mark_dirty(c, e);
}

const Rect& rect(RectangleRendererComponent& c, Entity e)
{
	return c.data.rect[hash::get(c.header.map, e)];
}

void set_color(RectangleRendererComponent& c, Entity e, const Color& color)
{
	c.data.color[hash::get(c.header.map, e)] = color;
	mark_dirty(c, e);
}

const Color& color(RectangleRendererComponent& c, Entity e)
{
	return c.data.color[hash::get(c.header.map, e)];
}

void set_render_handle(RectangleRendererComponent& c, Entity e, RenderResourceHandle render_handle)
{
	c.data.render_handle[hash::get(c.header.map, e)] = render_handle;
}

RenderResourceHandle material(RectangleRendererComponent& c, Entity e)
{
	return c.data.material[hash::get(c.header.map, e)];
}

void set_material(RectangleRendererComponent& c, Entity e, RenderResourceHandle material)
{
	c.data.material[hash::get(c.header.map, e)] = material;
}

RenderResourceHandle render_handle(RectangleRendererComponent& c, Entity e)
{
	return c.data.render_handle[hash::get(c.header.map, e)];
}

void set_geometry(RectangleRendererComponent& c, Entity e, const Quad& geometry)
{
	c.data.geometry[hash::get(c.header.map, e)] = geometry;
	mark_dirty(c, e);
}

const Quad& transform(RectangleRendererComponent& c, Entity e)
{
	return c.data.geometry[hash::get(c.header.map, e)];
}

RectangleRendererComponentData copy_data(RectangleRendererComponent& c, Entity e, Allocator& allocator)
{
	auto data = initialize_data(allocator.allocate(component_size), 1);
	unsigned i = hash::get(c.header.map, e);
	*data.color = c.data.color[i];
	*data.rect = c.data.rect[i];
	*data.material = c.data.material[i];
	*data.render_handle = c.data.render_handle[i];
	*data.geometry = c.data.geometry[i];
	return data;
}

RectangleRendererComponentData copy_dirty_data(RectangleRendererComponent& c, Allocator& allocator)
{
	auto num_dirty = c.header.last_dirty_index + 1;
	auto data = initialize_data(allocator.allocate(component_size), num_dirty);
	copy(c, data, num_dirty);
	return data;
}

RectangleRendererComponentData create_data_from_buffer(void* buffer, unsigned num)
{
	return initialize_data(buffer, num);
}

} // rectangle_renderer_component

} // namespace bowtie
