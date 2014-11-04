#pragma once

#include "rectangle_renderer_component.h"
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
	new_data.render_handle = (RenderResourceHandle*)(new_data.rect + size);
	return new_data;
}

void grow(RectangleRendererComponent& c, Allocator& allocator)
{
	const unsigned new_capacity = c.capacity == 0 ? 8 : c.capacity * 2;
	const unsigned bytes = new_capacity * rectangle_renderer_component::component_size;
	void* buffer = allocator.allocate(bytes);

	auto new_data = initialize_data(buffer, new_capacity);
	memcpy(new_data.color, c.data.color, c.num * sizeof(Color));
	memcpy(new_data.rect, c.data.rect, c.num * sizeof(Rect));
	memcpy(new_data.render_handle, c.data.render_handle, c.num * sizeof(RenderResourceHandle));
	c.data = new_data;

	allocator.deallocate(c.buffer);
	c.buffer = buffer;
	c.capacity = new_capacity;
}

void mark_dirty(RectangleRendererComponent& c, Entity e)
{
	auto entity_index = hash::get(c.map, e, 0u);

	if (c.last_dirty_index != (unsigned)-1 && entity_index <= c.last_dirty_index)
		return;

	auto current_dirty_index = ++c.last_dirty_index;

	if (current_dirty_index == entity_index)
		return;

	auto color_at_index = c.data.color[current_dirty_index];
	auto rect_at_index = c.data.rect[current_dirty_index];
	auto render_handle_at_index = c.data.render_handle[current_dirty_index];
	c.data.color[current_dirty_index] = c.data.color[entity_index];
	c.data.rect[current_dirty_index] = c.data.rect[entity_index];
	c.data.render_handle[current_dirty_index] = c.data.render_handle[entity_index];
	c.data.color[entity_index] = color_at_index;
	c.data.rect[entity_index] = rect_at_index;
	c.data.render_handle[entity_index] = render_handle_at_index;
}

}

namespace rectangle_renderer_component
{

void init(RectangleRendererComponent& c, Allocator& allocator)
{
	memset(&c, 0, sizeof(RectangleRendererComponent));
	c.map = hash::create<unsigned>(allocator);
	c.last_dirty_index = (unsigned)-1;
}

void deinit(RectangleRendererComponent& c, Allocator& allocator)
{
	hash::deinit(c.map);
	allocator.deallocate(c.buffer);
}

void create(RectangleRendererComponent& c, Entity e, Allocator& allocator)
{
	if (c.num >= c.capacity)
		grow(c, allocator);

	unsigned i = c.num++;
	hash::set(c.map, e, i);
	c.data.color[i] = Color(1, 1, 1, 1);
	c.data.rect[i] = Rect();
	c.data.render_handle[i] = RenderResourceHandle::NotInitialized;
}

void destroy(RectangleRendererComponent& c, Entity e)
{
	unsigned i = hash::get(c.map, e, 0u);
	hash::remove(c.map, e);
	--c.num;

	if (i == c.num)
		return;
		
	c.data.color[i] = c.data.color[c.num];
	c.data.rect[i] = c.data.rect[c.num];
	c.data.render_handle[i] = c.data.render_handle[c.num];
}

void set_rect(RectangleRendererComponent& c, Entity e, const Rect& rect)
{	
	c.data.rect[hash::get(c.map, e)] = rect;
	mark_dirty(c, e);
}

const Rect& rect(RectangleRendererComponent& c, Entity e)
{
	return c.data.rect[hash::get(c.map, e)];
}

void set_color(RectangleRendererComponent& c, Entity e, const Color& color)
{
	c.data.color[hash::get(c.map, e)] = color;
	mark_dirty(c, e);
}

const Color& color(RectangleRendererComponent& c, Entity e)
{
	return c.data.color[hash::get(c.map, e)];
}

void set_render_handle(RectangleRendererComponent& c, Entity e, RenderResourceHandle render_handle)
{
	c.data.render_handle[hash::get(c.map, e)] = render_handle;
}

RenderResourceHandle render_handle(RectangleRendererComponent& c, Entity e)
{
	return c.data.render_handle[hash::get(c.map, e)];
}

RectangleRendererComponentData* copy_data(RectangleRendererComponent& c, Entity e, Allocator& allocator)
{
	RectangleRendererComponentData* data = (RectangleRendererComponentData*)allocator.allocate(sizeof(RectangleRendererComponentData));
	*data = initialize_data(allocator.allocate(component_size), 1);
	*data->color = color(c, e);
	*data->rect = rect(c, e);
	*data->render_handle = render_handle(c, e);
	return data;
}

RectangleRendererComponentData* copy_dirty_data(RectangleRendererComponent& c, Allocator& allocator)
{
	RectangleRendererComponentData* data = (RectangleRendererComponentData*)allocator.allocate(sizeof(RectangleRendererComponentData));
	auto num_dirty = c.last_dirty_index + 1;
	*data = initialize_data(allocator.allocate(component_size), num_dirty);
	memcpy(data->color, c.data.color, sizeof(Color) * num_dirty);
	memcpy(data->rect, c.data.rect, sizeof(Rect) * num_dirty);
	memcpy(data->render_handle, c.data.render_handle, sizeof(RenderResourceHandle) * num_dirty);
	return data;
}

} // rectangle_renderer_component

} // namespace bowtie
