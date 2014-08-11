#include "render_world.h"

#include <foundation/array.h>
#include "render_target.h"
#include "render_drawable.h"

namespace bowtie
{

RenderWorld::RenderWorld(Allocator& allocator, RenderTarget& render_target) : _allocator(allocator), _drawables(allocator), _render_target(render_target)
{
}

RenderWorld::~RenderWorld()
{
	_allocator.destroy(&_render_target);
}

void RenderWorld::add_drawable(RenderDrawable* drawable)
{
	array::push_back(_drawables, drawable);
}

void RenderWorld::remove_drawable(RenderDrawable* drawable)
{
	array::remove(_drawables, drawable);
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