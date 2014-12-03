#pragma once

#include <foundation/collection_types.h>
#include "render_resource.h"
#include "render_target.h"

namespace bowtie
{

class Allocator;
struct Rect;
struct RenderComponent;

struct RenderWorld
{
	Array<RenderComponent*> components;
	RenderTarget render_target;
};

namespace render_world
{
	void init(RenderWorld& rw, const RenderTarget& render_target, Allocator& allocator);
	void deinit(RenderWorld& rw);
	void add_component(RenderWorld& rw, RenderComponent* component);
	void sort(RenderWorld& rw);
}

};