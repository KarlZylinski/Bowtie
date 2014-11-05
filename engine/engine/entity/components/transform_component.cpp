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
	new_data.position = (Vector2*)buffer;
	new_data.rotation = (float*)(new_data.position + size);
	new_data.pivot = (Vector2*)(new_data.position + size);
	return new_data;
}

void move_one(TransformComponent& c, unsigned from, unsigned to)
{
	c.data.position[to] = c.data.position[from];
	c.data.rotation[to] = c.data.rotation[from];
	c.data.pivot[to] = c.data.pivot[from];
}

void copy(TransformComponent& c, TransformComponentData& dest, unsigned num)
{
	memcpy(dest.position, c.data.position, num * sizeof(Vector2));
	memcpy(dest.rotation, c.data.rotation, num * sizeof(float));
	memcpy(dest.pivot, c.data.pivot, num * sizeof(Vector2));
}

void copy(TransformComponent& c, TransformComponentData& dest)
{
	copy(c, dest, c.header.num);
}

void grow(TransformComponent& c, Allocator& allocator)
{
	const unsigned new_capacity = c.header.capacity == 0 ? 8 : c.header.capacity * 2;
	const unsigned bytes = new_capacity * transform_component::component_size;
	void* buffer = allocator.allocate(bytes);

	auto new_data = initialize_data(buffer, new_capacity);
	copy(c, new_data);
	c.data = new_data;

	allocator.deallocate(c.buffer);
	c.buffer = buffer;
	c.header.capacity = new_capacity;
}

void mark_dirty(TransformComponent& c, Entity e)
{
	auto entity_index = hash::get(c.header.map, e, 0u);

	if (c.header.last_dirty_index != (unsigned)-1 && entity_index <= c.header.last_dirty_index)
		return;

	auto current_dirty_index = ++c.header.last_dirty_index;

	if (current_dirty_index == entity_index)
		return;

	auto position_at_index = c.data.position[current_dirty_index];
	auto rotation_at_index = c.data.rotation[current_dirty_index];
	auto pivot_at_index = c.data.pivot[current_dirty_index];
	move_one(c, entity_index, current_dirty_index);
	c.data.position[entity_index] = position_at_index;
	c.data.rotation[entity_index] = rotation_at_index;
	c.data.pivot[entity_index] = pivot_at_index;
}

}

namespace transform_component
{

unsigned component_size = (sizeof(Vector2) + sizeof(float) + sizeof(Vector2));

void init(TransformComponent& c, Allocator& allocator)
{
	memset(&c, 0, sizeof(TransformComponent));
	c.header.map = hash::create<unsigned>(allocator);
	c.header.last_dirty_index = (unsigned)-1;
}

void deinit(TransformComponent& c, Allocator& allocator)
{
	hash::deinit(c.header.map);
	allocator.deallocate(c.buffer);
}

void create(TransformComponent& c, Entity e, Allocator& allocator)
{
	if (c.header.num >= c.header.capacity)
		grow(c, allocator);

	unsigned i = c.header.num++;
	hash::set(c.header.map, e, i);
	c.data.position[i] = Vector2(0, 0);
	c.data.rotation[i] = 0;
	c.data.pivot[i] = Vector2(0, 0);
}

void destroy(TransformComponent& c, Entity e)
{
	unsigned i = hash::get(c.header.map, e, 0u);
	hash::remove(c.header.map, e);
	--c.header.num;

	if (i == c.header.num)
		return;
		
	move_one(c, c.header.num, i);
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

TransformComponentData* copy_dirty_data(TransformComponent& c, Allocator& allocator)
{
	TransformComponentData* data = (TransformComponentData*)allocator.allocate(sizeof(TransformComponentData));
	auto num_dirty = c.header.last_dirty_index + 1;
	*data = initialize_data(allocator.allocate(component_size), num_dirty);
	copy(c, *data, num_dirty);
	return data;
}

} // namespace transform_component

} // namespace bowtie
