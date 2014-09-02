#pragma once

#include "render_resource_handle.h"

namespace bowtie
{

struct Shader;
struct Material
{
	Material(RenderResourceHandle render_handle, Shader* shader) :
		render_handle(render_handle), shader(shader) {}
	RenderResourceHandle render_handle;
	Shader* shader;
};

}