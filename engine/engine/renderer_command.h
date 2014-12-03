#pragma once

#include "rect.h"
#include "render_resource_handle.h"
#include "uniform.h"
#include <foundation/vector2u.h>
#include <foundation/vector4.h>
#include <foundation/matrix4.h>
#include <stdint.h>

namespace bowtie
{

struct RendererCommand
{
	enum Type { Fence, RenderWorld, LoadResource, UpdateResource, Resize, CombineRenderedWorlds, SetUniformValue };
	Type type;
	void* data;
	unsigned dynamic_data_size;
	void* dynamic_data;
};

struct RenderWorldData
{
	Rect view;
	RenderResourceHandle render_world;
};

struct ResizeData
{
	Vector2u resolution;
};

struct SetUniformValueData
{
	RenderResourceHandle material;
	uniform::Type type;
	uint64_t uniform_name;
};

}