#pragma once

#include <foundation/matrix4.h>
#include <engine/resource_handle.h>

namespace bowtie
{

struct RenderDrawable
{
	ResourceHandle texture;
	ResourceHandle shader;
	ResourceHandle geometry;
	unsigned num_vertices;
	Matrix4 model;
};

}