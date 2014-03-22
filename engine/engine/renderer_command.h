#pragma once

#include "view.h"

namespace bowtie
{

struct RendererCommand
{
	enum Type { Fence, RenderWorld, LoadResource };
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

}