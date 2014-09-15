#pragma once

#include "view.h"
#include "render_resource_handle.h"
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
	RenderWorldData(const View& view, RenderResourceHandle& render_world) : view(view), render_world(render_world) {}

	View view;
	RenderResourceHandle render_world;
};

struct ResizeData
{
	ResizeData(const Vector2u& resolution) : resolution(resolution) {}

	Vector2u resolution;
};

struct DrawableStateReflectionData
{
	DrawableStateReflectionData(const Matrix4& model) : model(model) {}

	RenderResourceHandle drawble;
	RenderResourceHandle material;
	Matrix4 model;
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
	uint64_t uniform_name;
	Vector4 value;
};

}