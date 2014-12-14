#pragma once

#include "../../render_resource_handle.h"
#include "component_header.h"

namespace bowtie
{

struct Vector2;
struct Matrix4;

struct TransformComponentData
{
	Entity* entity;
	Vector2* position;
	real32* rotation;
	Vector2* pivot;
	uint32* parent;
	uint32* first_child;
	uint32* next_sibling;
	uint32* previous_sibling;
	Matrix4* world_transform;
};

struct TransformComponent
{
	ComponentHeader header;
	void* buffer;
	TransformComponentData data;
};

namespace transform_component
{
	const uint32 not_assigned = (uint32)-1;
	extern uint32 component_size;
	void init(TransformComponent* c, Allocator* allocator);
	void deinit(TransformComponent* c, Allocator* allocator);
	void create(TransformComponent* c, Entity e, Allocator* allocator);
	void destroy(TransformComponent* c, Entity e);
	void set_position(TransformComponent* c, Entity e, const Vector2* rect);
	const Vector2* position(TransformComponent* c, Entity e);
	void set_rotation(TransformComponent* c, Entity e, real32 rotation);
	real32 rotation(TransformComponent* c, Entity e);
	void set_pivot(TransformComponent* c, Entity e, const Vector2* pivot);
	const Vector2* pivot(TransformComponent* c, Entity e);
	void set_parent(TransformComponent* c, Entity e, Entity parent);
	Entity parent(TransformComponent* c, Entity e);
	void set_world_transform(TransformComponent* c, Entity e, const Matrix4* world_transform);
	const Matrix4* world_transform(TransformComponent* c, Entity e);
	void* copy_dirty_data(TransformComponent* c, Allocator* allocator);
}

}
