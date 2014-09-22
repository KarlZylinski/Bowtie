#pragma once

#include <foundation/matrix4.h>
#include "render_resource.h"

namespace bowtie
{

class RenderMaterial;
struct RenderTexture;

struct RenderDrawable
{
	RenderTexture* texture;
	RenderMaterial* material;
	RenderResource geometry;
	unsigned num_vertices;
	Matrix4 model;
	int depth;
};

}