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
	float* rotation;
	Vector2* pivot;
	unsigned* parent;
	unsigned* first_child;
	unsigned* next_sibling;
	unsigned* previous_sibling;
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
	const unsigned not_assigned = (unsigned)-1;
	extern unsigned component_size;
	void init(TransformComponent* c, Allocator* allocator);
	void deinit(TransformComponent* c, Allocator* allocator);
	void create(TransformComponent* c, Entity e, Allocator* allocator);
	void destroy(TransformComponent* c, Entity e);
	void set_position(TransformComponent* c, Entity e, const Vector2* rect);
	const Vector2* position(TransformComponent* c, Entity e);
	void set_rotation(TransformComponent* c, Entity e, float rotation);
	float rotation(TransformComponent* c, Entity e);
	void set_pivot(TransformComponent* c, Entity e, const Vector2* pivot);
	const Vector2* pivot(TransformComponent* c, Entity e);
	void set_parent(TransformComponent* c, Entity e, Entity parent);
	Entity parent(TransformComponent* c, Entity e);
	void set_world_transform(TransformComponent* c, Entity e, const Matrix4* world_transform);
	const Matrix4* world_transform(TransformComponent* c, Entity e);
	void* copy_dirty_data(TransformComponent* c, Allocator* allocator);
}

}
