#pragma once

#include <foundation/vector4.h>
#include <foundation/matrix4.h>
#include <foundation/quad.h>

namespace bowtie
{

struct RenderComponent
{
	RenderResourceHandle material;
	Color color;
	Quad geometry;
	int depth;
};

}