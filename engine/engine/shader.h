#pragma once

#include "render_resource_handle.h"

namespace bowtie
{

struct Shader
{
	Shader(RenderResourceHandle render_handle) : render_handle(render_handle) {}
	RenderResourceHandle render_handle;
};

}