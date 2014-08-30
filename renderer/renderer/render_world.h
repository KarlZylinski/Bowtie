#pragma once

#include <foundation/collection_types.h>

#include "render_resource.h"

namespace bowtie
{

class Allocator;
struct RenderTarget;
struct RenderDrawable;
class RenderWorld
{
public:
	RenderWorld(Allocator& allocator, RenderTarget& render_target);
	~RenderWorld();

	void add_drawable(RenderDrawable* drawable);
	void remove_drawable(RenderDrawable* drawable);
	const RenderTarget& render_target() const;
	RenderTarget& render_target();
	const Array<RenderDrawable*>& drawables() const;
private:
	RenderWorld& operator=(const RenderWorld&);
	RenderWorld(const RenderWorld&);

	Allocator& _allocator;
	Array<RenderDrawable*> _drawables;
	RenderTarget& _render_target;
};

};