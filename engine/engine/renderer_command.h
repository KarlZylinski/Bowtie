#pragma once

#include "view.h"
#include "render_resource_handle.h"
#include "uniform.h"
#include <foundation/vector2u.h>
#include <foundation/vector4.h>
#include <stdint.h>

namespace bowtie
{

struct RendererCommand
{
	enum Type { Fence, RenderWorld, LoadResource, UpdateResource, Unspawn, Resize, DrawableStateReflection, DrawableGeometryReflection, CombineRenderedWorlds, SetUniformValue};
	Type type;
	void* data;
	unsigned dynamic_data_size;
	void* dynamic_data;
};

struct RenderWorldData
{
	View view;
	RenderResourceHandle render_world;
};

struct ResizeData
{
	Vector2u resolution;
};

struct DrawableStateReflectionData
{
	RenderResourceHandle drawble;
	RenderResourceHandle material;
	Matrix4 model;
	int depth;
};

struct DrawableGeometryReflectionData
{
	RenderResourceHandle drawable;
	unsigned size;
};

struct UnspawnData
{
	RenderResourceHandle world;
	RenderResourceHandle drawable;
};

struct SetUniformValueData
{
	RenderResourceHandle material;
	uniform::Type type;
	uint64_t uniform_name;
};

}