#pragma once

#include <foundation/collection_types.h>
#include "render_resource.h"
#include "render_target.h"

namespace bowtie
{

class Allocator;
struct RenderDrawable;
struct Rect;

struct RenderWorld
{
	Array<RenderDrawable*> drawables;
	Array<Rect> drawable_rects;
	RenderTarget render_target;
};

namespace render_world
{
	void init(RenderWorld& rw, const RenderTarget& render_target, Allocator& allocator);
	void deinit(RenderWorld& rw);
	void add_drawable(RenderWorld& rw, RenderDrawable* drawable);
	void remove_drawable(RenderWorld& rw, RenderDrawable* drawable);
	void add_drawable_rect(RenderWorld& rw, const Rect& rect);
	void sort(RenderWorld& rw);
}

};