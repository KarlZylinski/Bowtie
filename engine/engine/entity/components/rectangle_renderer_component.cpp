#pragma once

#include "rectangle_renderer_component.h"
#include <cassert>

namespace bowtie
{

namespace
{

void grow(RectangleRendererComponent& c, Allocator& allocator)
{
	const unsigned new_capacity = c.capacity == 0 ? 8 : c.capacity * 2;
	const unsigned bytes = new_capacity * (sizeof(Color) + sizeof(Rect) + sizeof(RenderResourceHandle));
	void* buffer = allocator.allocate(bytes);

	Color* color = (Color*)buffer;
	Rect* rect = (Rect*)(color + new_capacity);
	RenderResourceHandle* render_handle = (RenderResourceHandle*)(rect + new_capacity);

	memcpy(color, c.color, c.num * sizeof(Color));
	memcpy(rect, c.rect, c.num * sizeof(Rect));
	memcpy(render_handle, c.render_handle, c.num * sizeof(RenderResourceHandle));

	c.color = color;
	c.rect = rect;
	c.render_handle = render_handle;

	allocator.deallocate(c.buffer);
	c.buffer = buffer;
	c.capacity = new_capacity;
}

}

namespace rectangle_renderer_component
{

void init(RectangleRendererComponent& c, Allocator& allocator)
{
	memset(&c, 0, sizeof(RectangleRendererComponent));
	c.map = hash::create<unsigned>(allocator);
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

	unsigned i = ++c.num;
	hash::set(c.map, e, i);
	c.color[i] = Color(1, 1, 1, 1);
	c.rect[i] = Rect();
	c.render_handle[i] = RenderResourceHandle::NotInitialized;
}

void set_rect(RectangleRendererComponent& c, Entity e, const Rect& rect)
{
	unsigned i = hash::get(c.map, e, 0u);
	assert(i != 0);
	c.rect[i] = rect;
}

const Rect& rect(RectangleRendererComponent& c, Entity e)
{
	unsigned i = hash::get(c.map, e, 0u);
	assert(i != 0);
	return c.rect[i];
}

} // rectangle_renderer_component

} // namespace bowtie
