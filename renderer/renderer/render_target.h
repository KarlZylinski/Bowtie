#pragma once

#include "render_resource.h"

namespace bowtie
{

class Allocator;
struct RenderTexture;
struct RenderTarget
{
	RenderTarget(Allocator& allocator, RenderTexture* render_texture, RenderResource target_handle);
	~RenderTarget();
	Allocator& allocator;
	RenderTexture* render_texture;
	RenderResource target_handle;
	RenderTarget& operator=(const RenderTarget&);
};


}