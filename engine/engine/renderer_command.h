#pragma once

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

}