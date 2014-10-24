#pragma once

#include <foundation/collection_types.h>
#include "render_resource.h"
#include "render_target.h"

namespace bowtie
{

class Allocator;
struct RenderDrawable;

struct RenderWorld
{
	Array<RenderDrawable*> drawables;
	RenderTarget render_target;
};

namespace render_world
{
	void init(RenderWorld& rw, const RenderTarget& render_target, Allocator& allocator);
	void deinit(RenderWorld& rw);
	void add_drawable(RenderWorld& rw, RenderDrawable* drawable);
	void remove_drawable(RenderWorld& rw, RenderDrawable* drawable);
	void sort(RenderWorld& rw);
}

};