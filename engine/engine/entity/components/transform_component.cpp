#pragma once

#include "transform_component.h"
#include <foundation/vector2.h>
#include <cassert>

namespace bowtie
{

namespace
{

const unsigned not_assigned = (unsigned)-1;

TransformComponentData initialize_data(void* buffer, unsigned size)
{
	TransformComponentData new_data;
	new_data.entity = (Entity*)buffer;
	new_data.position = (Vector2*)(new_data.entity + size);
	new_data.rotation = (float*)(new_data.position + size);
	new_data.pivot = (Vector2*)(new_data.rotation + size);
	new_data.parent = (unsigned*)(new_data.pivot + size);
	new_data.first_child = (unsigned*)(new_data.parent + size);
	new_data.next_sibling = (unsigned*)(new_data.first_child + size);
	new_data.previous_sibling = (unsigned*)(new_data.next_sibling + size);
	return new_data;
}

void copy_offset(TransformComponentData& from, TransformComponentData& to, unsigned num, unsigned from_offset, unsigned to_offset)
{
	memcpy(to.entity + to_offset, from.entity + from_offset, num * sizeof(Entity));
	memcpy(to.position + to_offset, from.position + from_offset, num * sizeof(Vector2));
	memcpy(to.rotation + to_offset, from.rotation + from_offset, num * sizeof(float));
	memcpy(to.pivot + to_offset, from.pivot + from_offset, num * sizeof(Vector2));
	memcpy(to.parent + to_offset, from.parent + from_offset, num * sizeof(unsigned));
	memcpy(to.first_child + to_offset, from.first_child + from_offset, num * sizeof(unsigned));
	memcpy(to.next_sibling + to_offset, from.next_sibling + from_offset, num * sizeof(unsigned));
	memcpy(to.previous_sibling + to_offset, from.previous_sibling + from_offset, num * sizeof(unsigned));
}

void copy(TransformComponentData& from, TransformComponentData& to, unsigned num)
{
	copy_offset(from, to, num, 0, 0);
}

void internal_copy(TransformComponentData& c, unsigned from, unsigned to)
{
	copy_offset(c, c, 1, from, to);
}

void set_parent_internal(TransformComponentData& d, unsigned index, unsigned parent_index)
{
	// Remove any references to this transform from old parent and siblings.
	if (d.parent[index] != not_assigned)
	{
		auto old_parent = d.parent[index];
		auto old_parent_child_iter = d.first_child[old_parent];

		while (old_parent_child_iter != not_assigned)
		{
			if (old_parent_child_iter == index)
			{
				old_parent_child_iter = d.next_sibling[old_parent_child_iter];
				continue;
			}

			auto next = d.next_sibling[old_parent_child_iter];
			auto previous = d.previous_sibling[old_parent_child_iter];

			if (next != not_assigned && previous != not_assigned) // Removed from middle.
			{
				d.next_sibling[previous] = next;
				d.previous_sibling[next] = previous;
			}
			else if (next != not_assigned) // Removed from start.
				d.previous_sibling[next] = not_assigned;
			else if (previous != not_assigned) // Removed from end.
				d.next_sibling[previous] = not_assigned;

			break;
		}

		if (d.first_child[old_parent] == index)
			d.first_child[old_parent] = d.next_sibling[index];
	}

	if (parent_index != not_assigned)
	{
		// Parent already has children, insert at front of list.
		if (d.first_child[parent_index] != not_assigned)
			d.previous_sibling[d.first_child[parent_index]] = index;

		d.first_child[parent_index] = index;
	}

	d.parent[index] = parent_index;
}


void swap(TransformComponent& c, unsigned i1, unsigned i2)
{
	auto i1_parent = c.data.parent[i1];
	auto i2_parent = c.data.parent[i2];

	if (i1_parent != not_assigned)
		set_parent_internal(c.data, i1, not_assigned);

	if (i2_parent != not_assigned)
		set_parent_internal(c.data, i2, not_assigned);

	hash::set(c.header.map, c.data.entity[i1], i2);
	hash::set(c.header.map, c.data.entity[i2], i1);
	internal_copy(c.data, i2, c.header.num);
	internal_copy(c.data, i1, i2);
	internal_copy(c.data, c.header.num, i1);

	if (i1_parent != not_assigned)
		set_parent_internal(c.data, i2, i1_parent);

	if (i2_parent != not_assigned)
		set_parent_internal(c.data, i1, i2_parent);
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

void mark_dirty(TransformComponent& c, unsigned index)
{
	auto dd = component::mark_dirty(c.header, index);

	if (dd.new_index != dd.old_index)
		swap(c, dd.old_index, dd.new_index);
	
	auto child_iter = c.data.first_child[dd.new_index];

	while (child_iter != not_assigned)
	{
		auto entity = c.data.entity[child_iter];
		mark_dirty(c, child_iter);
		child_iter = hash::get(c.header.map, entity); // Index might change when swapping in recursive call.
		auto child_parent = c.data.parent[child_iter];
		assert(c.data.parent[child_iter] != not_assigned);

		if (child_iter < child_parent)
			swap(c, child_iter, child_parent); // Parents must be before all children, otherwise updating will be wonky.

		child_iter = c.data.next_sibling[child_iter];
	}
}

}

namespace transform_component
{

	unsigned component_size = sizeof(Entity) + sizeof(Vector2) + sizeof(float) + sizeof(Vector2)
							  + sizeof(unsigned) + sizeof(unsigned) + sizeof(unsigned) + sizeof(unsigned);

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
	c.data.parent[i] = not_assigned;
	c.data.first_child[i] = not_assigned;
	c.data.next_sibling[i] = not_assigned;
	c.data.previous_sibling[i] = not_assigned;

	if (c.header.first_new == not_assigned)
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
	auto i = hash::get(c.header.map, e);
	c.data.position[i] = position;
	mark_dirty(c, i);
}

const Vector2& position(TransformComponent& c, Entity e)
{
	return c.data.position[hash::get(c.header.map, e)];
}

void set_rotation(TransformComponent& c, Entity e, float rotation)
{
	auto i = hash::get(c.header.map, e);
	c.data.rotation[hash::get(c.header.map, e)] = rotation;
	mark_dirty(c, i);
}

float rotation(TransformComponent& c, Entity e)
{
	return c.data.rotation[hash::get(c.header.map, e)];
}

void set_pivot(TransformComponent& c, Entity e, const Vector2& pivot)
{
	auto i = hash::get(c.header.map, e);
	c.data.pivot[hash::get(c.header.map, e)] = pivot;
	mark_dirty(c, i);
}

const Vector2& pivot(TransformComponent& c, Entity e)
{
	return c.data.pivot[hash::get(c.header.map, e)];
}

void set_parent(TransformComponent& c, Entity e, Entity parent_entity)
{
	auto i = hash::get(c.header.map, e);
	set_parent_internal(c.data, i, hash::get(c.header.map, parent_entity));
	mark_dirty(c, i);
}

Entity parent(TransformComponent& c, Entity e)
{
	return c.data.parent[hash::get(c.header.map, e)];
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
