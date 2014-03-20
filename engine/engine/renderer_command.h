#pragma once

namespace bowtie
{

struct RendererCommand
{
	enum Type { Fence, RenderWorld };
	Type type;
	void* data;
};

}