#pragma once

#include <foundation/vector2u.h>

#include "resource_handle.h"
#include "view.h"

namespace bowtie
{

struct RendererCommand
{
	enum Type { Fence, RenderWorld, LoadResource, Resize, SetUpSpriteRenderingQuad };
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

}