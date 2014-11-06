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
	new_data.entity = (Entity*)buffer;
	new_data.color = (Color*)(new_data.entity + size);
	new_data.rect = (Rect*)(new_data.color + size);
	new_data.material = (RenderResourceHandle*)(new_data.rect + size);
	new_data.render_handle = (RenderResourceHandle*)(new_data.material + size);
	new_data.geometry = (Quad*)(new_data.render_handle + size);
	return new_data;
}

void move_one(RectangleRendererComponent& c, unsigned from, unsigned to)
{
	c.data.entity[to] = c.data.entity[from];
	c.data.color[to] = c.data.color[from];
	c.data.rect[to] = c.data.rect[from];
	c.data.material[to] = c.data.material[from];
	c.data.render_handle[to] = c.data.render_handle[from];
	c.data.geometry[to] = c.data.geometry[from];
}

void copy_from_offset(RectangleRendererComponent& c, RectangleRendererComponentData& dest, unsigned num, unsigned offset)
{
	memcpy(dest.entity, c.data.entity + offset, num * sizeof(Entity));
	memcpy(dest.color, c.data.color + offset, num * sizeof(Color));
	memcpy(dest.rect, c.data.rect + offset, num * sizeof(Rect));
	memcpy(dest.material, c.data.material + offset, num * sizeof(Material));
	memcpy(dest.render_handle, c.data.render_handle + offset, num * sizeof(RenderResourceHandle));
	memcpy(dest.geometry, c.data.geometry + offset, num * sizeof(Quad));
}

void copy(RectangleRendererComponent& c, RectangleRendererComponentData& dest, unsigned num)
{
	copy_from_offset(c, dest, num, 0);
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
	auto dd = component::mark_dirty(c.header, e);

	if (dd.new_index == dd.old_index)
		return;

	hash::set(c.header.map, e, dd.new_index);
	hash::set(c.header.map, c.data.entity[dd.new_index], dd.old_index);
	auto enity_at_index = c.data.entity[dd.new_index];
	auto color_at_index = c.data.color[dd.new_index];
	auto rect_at_index = c.data.rect[dd.new_index];
	auto material_at_index = c.data.material[dd.new_index];
	auto render_handle_at_index = c.data.render_handle[dd.new_index];
	auto transform_at_index = c.data.geometry[dd.new_index];
	move_one(c, dd.old_index, dd.new_index);
	c.data.entity[dd.old_index] = enity_at_index;
	c.data.color[dd.old_index] = color_at_index;
	c.data.rect[dd.old_index] = rect_at_index;
	c.data.material[dd.old_index] = material_at_index;
	c.data.render_handle[dd.old_index] = render_handle_at_index;
	c.data.geometry[dd.old_index] = transform_at_index;
}

}

namespace rectangle_renderer_component
{

unsigned component_size = (sizeof(Entity) + sizeof(Color) + sizeof(Rect) + sizeof(RenderResourceHandle) + sizeof(RenderResourceHandle) + sizeof(Quad));

void init(RectangleRendererComponent& c, Allocator& allocator)
{
	memset(&c, 0, sizeof(RectangleRendererComponent));
	component::init(c.header, allocator);
}

void deinit(RectangleRendererComponent& c, Allocator& allocator)
{
	component::deinit(c.header);
	allocator.deallocate(c.buffer);
}

void create(RectangleRendererComponent& c, Entity e, Allocator& allocator, const Rect& rect, const Color& color)
{
	if (c.header.num >= c.header.capacity)
		grow(c, allocator);

	unsigned i = c.header.num++;
	hash::set(c.header.map, e, i);
	c.data.entity[i] = e;
	c.data.color[i] = color;
	c.data.rect[i] = rect;
	c.data.material[i] = RenderResourceHandle::NotInitialized;
	c.data.render_handle[i] = RenderResourceHandle::NotInitialized;
	memset(c.data.geometry + i, 0, sizeof(Quad));
	
	if (c.header.first_new == (unsigned)-1)
		c.header.first_new = i;
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

void* copy_dirty_data(RectangleRendererComponent& c, Allocator& allocator)
{
	auto num_dirty = component::num_dirty(c.header);
	void* buffer = allocator.allocate(component_size * num_dirty);
	auto data = initialize_data(buffer, num_dirty);
	copy(c, data, num_dirty);
	return buffer;
}

void* copy_new_data(RectangleRendererComponent& c, Allocator& allocator)
{
	auto num_new = component::num_new(c.header);
	void* buffer = allocator.allocate(component_size * num_new);
	auto data = initialize_data(buffer, num_new);
	copy_from_offset(c, data, num_new, c.header.first_new);
	return buffer;
}

RectangleRendererComponentData create_data_from_buffer(void* buffer, unsigned num)
{
	return initialize_data(buffer, num);
}

} // rectangle_renderer_component

} // namespace bowtie
