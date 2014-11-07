#pragma once

#include "transform_component.h"
#include <foundation/vector2.h>
#include <cassert>

namespace bowtie
{

namespace
{

TransformComponentData initialize_data(void* buffer, unsigned size)
{
	TransformComponentData new_data;
	new_data.entity = (Entity*)buffer;
	new_data.position = (Vector2*)(new_data.entity + size);
	new_data.rotation = (float*)(new_data.position + size);
	new_data.pivot = (Vector2*)(new_data.rotation + size);
	return new_data;
}

void copy_offset(TransformComponentData& from, TransformComponentData& to, unsigned num, unsigned from_offset, unsigned to_offset)
{
	memcpy(to.entity + to_offset, from.entity + from_offset, num * sizeof(Entity));
	memcpy(to.position + to_offset, from.position + from_offset, num * sizeof(Vector2));
	memcpy(to.rotation + to_offset, from.rotation + from_offset, num * sizeof(float));
	memcpy(to.pivot + to_offset, from.pivot + from_offset, num * sizeof(Vector2));
}

void copy(TransformComponentData& from, TransformComponentData& to, unsigned num)
{
	copy_offset(from, to, num, 0, 0);
}

void internal_copy(TransformComponentData& c, unsigned from, unsigned to)
{
	copy_offset(c, c, 1, from, to);
}

void grow(TransformComponent& c, Allocator& allocator)
{
	const unsigned new_capacity = (c.header.capacity == 0 ? 8 : c.header.capacity * 2) + 1; // One extra so last index always can be used for swapping.
	const unsigned bytes = new_capacity * transform_component::component_size;
	void* buffer = allocator.allocate(bytes);

	auto new_data = initialize_data(buffer, new_capacity);
	copy(c.data, new_data, c.header.num);
	c.data = new_data;

	allocator.deallocate(c.buffer);
	c.buffer = buffer;
	c.header.capacity = new_capacity;
}

void mark_dirty(TransformComponent& c, Entity e)
{
	auto dd = component::mark_dirty(c.header, e);

	if (dd.new_index == dd.old_index)
		return;
		
	hash::set(c.header.map, e, dd.new_index);
	hash::set(c.header.map, c.data.entity[dd.new_index], dd.old_index);
	internal_copy(c.data, dd.new_index, c.header.num);
	internal_copy(c.data, dd.old_index, dd.new_index);
	internal_copy(c.data, c.header.num, dd.old_index);
}

}

namespace transform_component
{

unsigned component_size = (sizeof(Entity) + sizeof(Vector2) + sizeof(float) + sizeof(Vector2));

void init(TransformComponent& c, Allocator& allocator)
{
	memset(&c, 0, sizeof(TransformComponent));
	component::init(c.header, allocator);
}

void deinit(TransformComponent& c, Allocator& allocator)
{
	component::deinit(c.header);
	allocator.deallocate(c.buffer);
}

void create(TransformComponent& c, Entity e, Allocator& allocator)
{
	if (c.header.num >= c.header.capacity)
		grow(c, allocator);

	unsigned i = c.header.num++;
	hash::set(c.header.map, e, i);
	c.data.entity[i] = e;
	c.data.position[i] = Vector2(0, 0);
	c.data.rotation[i] = 0;
	c.data.pivot[i] = Vector2(0, 0);

	if (c.header.first_new == (unsigned)-1)
		c.header.first_new = i;
}

void destroy(TransformComponent& c, Entity e)
{
	unsigned i = hash::get(c.header.map, e, 0u);
	hash::remove(c.header.map, e);
	--c.header.num;

	if (i == c.header.num)
		return;
		
	internal_copy(c.data, c.header.num - 1, i);
}

void set_position(TransformComponent& c, Entity e, const Vector2& position)
{	
	c.data.position[hash::get(c.header.map, e)] = position;
	mark_dirty(c, e);
}

const Vector2& position(TransformComponent& c, Entity e)
{
	return c.data.position[hash::get(c.header.map, e)];
}

void set_rotation(TransformComponent& c, Entity e, float rotation)
{
	c.data.rotation[hash::get(c.header.map, e)] = rotation;
	mark_dirty(c, e);
}

float rotation(TransformComponent& c, Entity e)
{
	return c.data.rotation[hash::get(c.header.map, e)];
}

void set_pivot(TransformComponent& c, Entity e, const Vector2& pivot)
{
	c.data.pivot[hash::get(c.header.map, e)] = pivot;
	mark_dirty(c, e);
}

const Vector2& pivot(TransformComponent& c, Entity e)
{
	return c.data.pivot[hash::get(c.header.map, e)];
}

void* copy_dirty_data(TransformComponent& c, Allocator& allocator)
{
	auto num_dirty = c.header.last_dirty_index + 1;
	void* buffer = allocator.allocate(component_size * num_dirty);
	auto data = initialize_data(buffer, num_dirty);
	copy(c.data, data, num_dirty);
	return buffer;
}

} // namespace transform_component

} // namespace bowtie
