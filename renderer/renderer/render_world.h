#pragma once

#include <foundation/collection_types.h>

#include "render_resource_handle.h"

namespace bowtie
{

class Allocator;
struct RenderTarget;
class RenderWorld
{
public:
	RenderWorld(Allocator& allocator, RenderTarget& render_target);
	~RenderWorld();

	void add_drawable(RenderResourceHandle drawable);
	void remove_drawable(RenderResourceHandle drawable);
	const RenderTarget& render_target() const;
	RenderTarget& render_target();
	const Array<RenderResourceHandle>& drawables() const;
private:
	RenderWorld& operator=(const RenderWorld&);
	RenderWorld(const RenderWorld&);

	Allocator& _allocator;
	Array<RenderResourceHandle> _drawables;
	RenderTarget& _render_target;
};

};