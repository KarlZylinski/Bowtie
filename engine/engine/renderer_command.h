#pragma once

#include "view.h"
#include "resource_handle.h"
#include <foundation/vector2u.h>

namespace bowtie
{

struct RendererCommand
{
	enum Type { Fence, RenderWorld, LoadResource, Unspawn, Resize, DrawableStateReflection, DrawableGeometryReflection, CombineRenderedWorlds };
	Type type;
	void* data;
	unsigned dynamic_data_size;
	void* dynamic_data;
};

struct RenderWorldData
{
	RenderWorldData(const View& view, ResourceHandle& render_world) : view(view), render_world(render_world) {}

	View view;
	ResourceHandle render_world;
};

struct ResizeData
{
	ResizeData(const Vector2u& resolution) : resolution(resolution) {}

	Vector2u resolution;
};

struct DrawableStateReflectionData
{
	DrawableStateReflectionData(const Matrix4& model) : model(model) {}

	ResourceHandle drawble;
	Matrix4 model;
};

struct DrawableGeometryReflectionData
{
	ResourceHandle drawable;
	unsigned size;
};

struct UnspawnData
{
	ResourceHandle world;
	ResourceHandle drawable;
};
}