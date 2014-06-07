#pragma once

#include <foundation/matrix4.h>
#include "resource_handle.h"

namespace bowtie
{

struct RenderSprite
{
	ResourceHandle texture;
	ResourceHandle shader;
	ResourceHandle geometry;
	Matrix4 model;
};

}