#pragma once

#include <foundation/matrix4.h>
#include <engine/render_resource_handle.h>

namespace bowtie
{

class RenderMaterial;
struct RenderTexture;

struct RenderDrawable
{
	RenderResourceHandle texture;
	RenderResourceHandle material;
	RenderResourceHandle geometry;
	unsigned num_vertices;
	Matrix4 model;
	int depth;
};

}