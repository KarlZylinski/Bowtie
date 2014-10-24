#include "render_world.h"

#include <foundation/array.h>
#include "render_target.h"
#include "render_drawable.h"
#include <algorithm>

namespace bowtie
{

RenderWorld::RenderWorld(Allocator& allocator, RenderTarget& render_target) : _allocator(allocator), _drawables(array::create<RenderDrawable*>(allocator)), _render_target(render_target)
{
}

RenderWorld::~RenderWorld()
{
	_allocator.deallocate(_render_target.texture.object);
	_allocator.deallocate(&_render_target);
	array::deinit(_drawables);
}

void RenderWorld::add_drawable(RenderDrawable* drawable)
{
	array::push_back(_drawables, drawable);
}

void RenderWorld::remove_drawable(RenderDrawable* drawable)
{
	array::remove(_drawables, drawable);
}

void RenderWorld::sort()
{
	std::sort(&_drawables[0], &_drawables[array::size(_drawables)], [](RenderDrawable* x, RenderDrawable* y){ return x->depth < y->depth; });
}

const RenderTarget& RenderWorld::render_target() const
{
	return _render_target;
}

RenderTarget& RenderWorld::render_target()
{
	return _render_target;
}

const Array<RenderDrawable*>& RenderWorld::drawables() const
{
	return _drawables;
}

} // namespace bowtie