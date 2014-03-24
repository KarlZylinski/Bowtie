#pragma once

#include "view.h"
#include <foundation/vector2u.h>

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
	RenderWorldData(const View& view) : view(view) {}

	View view;
};

struct ResizeData
{
	ResizeData(const Vector2u& resolution) : resolution(resolution) {}

	Vector2u resolution;
};

}