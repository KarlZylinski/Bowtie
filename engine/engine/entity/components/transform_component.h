#pragma once

#include "../../render_resource_handle.h"
#include "component_header.h"

namespace bowtie
{

struct Vector2;

struct TransformComponentData
{
	Entity* entity;
	Vector2* position;
	float* rotation;
	Vector2* pivot;
};

struct TransformComponent
{
	ComponentHeader header;
	void* buffer;
	TransformComponentData data;
};

namespace transform_component
{
	extern unsigned component_size;
	void init(TransformComponent& c, Allocator& allocator);
	void deinit(TransformComponent& c, Allocator& allocator);
	void create(TransformComponent& c, Entity e, Allocator& allocator);
	void destroy(TransformComponent& c, Entity e);
	void set_position(TransformComponent& c, Entity e, const Vector2& rect);
	const Vector2& position(TransformComponent& c, Entity e);
	void set_rotation(TransformComponent& c, Entity e, float rotation);
	float rotation(TransformComponent& c, Entity e);
	TransformComponentData* copy_dirty_data(TransformComponent& c, Allocator& allocator);
}

}
