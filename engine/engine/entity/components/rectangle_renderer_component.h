#pragma once

#include <foundation/vector4.h>
#include <foundation/hash.h>
#include "../../rect.h"
#include "../../render_resource_handle.h"

namespace bowtie
{

class Allocator;
class RenderInterface;

typedef unsigned Entity;

struct RectangleRendererComponent
{
	// Maps entity id to rectangle renderer components
	Hash<unsigned> map;
	unsigned num;
	unsigned capacity;
	void* buffer;
	Color* color;
	Rect* rect;
	RenderResourceHandle* render_handle;
};

namespace rectangle_renderer_component
{
	void init(RectangleRendererComponent& c, Allocator& allocator);
	void deinit(RectangleRendererComponent& c, Allocator& allocator);
	void create(RectangleRendererComponent& c, Entity e, Allocator& allocator);
	void set_rect(RectangleRendererComponent& c, Entity e, const Rect& rect);
	const Rect& rect(RectangleRendererComponent& c, Entity e);
}

}