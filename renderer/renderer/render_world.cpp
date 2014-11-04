#include "render_world.h"
#include <foundation/array.h>
#include <engine/rect.h>
#include "render_target.h"
#include "render_drawable.h"
#include <algorithm>

namespace bowtie
{

namespace render_world
{

void init(RenderWorld& rw, const RenderTarget& render_target, Allocator& allocator)
{
	rw.drawables = array::create<RenderDrawable*>(allocator);
	rw.drawable_rects = array::create<Rect*>(allocator);
	rw.render_target = render_target;
}

void deinit(RenderWorld& rw)
{
	array::deinit(rw.drawables);
	array::deinit(rw.drawable_rects);
}

void add_drawable(RenderWorld& rw, RenderDrawable* drawable)
{
	array::push_back(rw.drawables, drawable);
}

void remove_drawable(RenderWorld& rw, RenderDrawable* drawable)
{
	array::remove(rw.drawables, drawable);
}

void add_drawable_rect(RenderWorld& rw, Rect* rect)
{
	array::push_back(rw.drawable_rects, rect);	
}

void sort(RenderWorld& rw)
{
	std::sort(&rw.drawables[0], &rw.drawables[array::size(rw.drawables)], [](RenderDrawable* x, RenderDrawable* y){ return x->depth < y->depth; });
}

} // namespace render_world

} // namespace bowtie