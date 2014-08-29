#pragma once

#include <foundation/matrix4.h>
#include "render_resource_handle.h"

namespace bowtie
{

class Material;
struct RenderTexture;

struct RenderDrawable
{
	RenderTexture* texture;
	Material* material;
	RenderResourceHandle geometry;
	unsigned num_vertices;
	Matrix4 model;
};

}