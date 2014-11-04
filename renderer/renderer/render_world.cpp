#include "render_world.h"
#include <foundation/array.h>
#include <engine/rect.h>
#include "render_target.h"
#include "render_drawable.h"
#include "render_component.h"
#include <algorithm>

namespace bowtie
{

namespace render_world
{

void init(RenderWorld& rw, const RenderTarget& render_target, Allocator& allocator)
{
	rw.drawables = array::create<RenderDrawable*>(allocator);
	rw.components = array::create<RenderComponent*>(allocator);
	rw.render_target = render_target;
}

void deinit(RenderWorld& rw)
{
	array::deinit(rw.drawables);
	array::deinit(rw.components);
}

void add_drawable(RenderWorld& rw, RenderDrawable* drawable)
{
	array::push_back(rw.drawables, drawable);
}

void remove_drawable(RenderWorld& rw, RenderDrawable* drawable)
{
	array::remove(rw.drawables, drawable);
}

void add_component(RenderWorld& rw, RenderComponent* component)
{
	array::push_back(rw.components, component);
}

void sort(RenderWorld& rw)
{
	std::sort(&rw.drawables[0], &rw.drawables[array::size(rw.drawables)], [](RenderDrawable* x, RenderDrawable* y){ return x->depth < y->depth; });
	std::sort(&rw.components[0], &rw.components[array::size(rw.components)], [](RenderComponent* x, RenderComponent* y){ return x->material < y->material; });
}

} // namespace render_world

} // namespace bowtie