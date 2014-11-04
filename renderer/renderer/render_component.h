#pragma once

#include <engine/rect.h>
#include <foundation/vector4.h>

namespace bowtie
{

struct RenderComponent
{
	RenderResourceHandle material;
	Color color;
	Rect rect;
};

}