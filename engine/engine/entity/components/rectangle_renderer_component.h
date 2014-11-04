#pragma once

#include <foundation/hash.h>
#include "../../render_resource_handle.h"

namespace bowtie
{

class Allocator;
class RenderInterface;
struct Material;
struct Rect;
struct Vector4;
typedef Vector4 Color;
typedef unsigned Entity;

struct RectangleRendererComponentData
{
	Color* color;
	Rect* rect;
	RenderResourceHandle* material;
	RenderResourceHandle* render_handle;
};

struct RectangleRendererComponent
{
	// Maps entity id to rectangle renderer components
	Hash<unsigned> map;
	unsigned num;
	unsigned capacity;
	// Maybe skip dirtyness and do statereflection right away? But if I can get away with doing deferred reflection
	// checks on only the touched ones, then maybe it's alright.
	unsigned last_dirty_index;
	void* buffer;
	RectangleRendererComponentData data;
};

namespace rectangle_renderer_component
{
	extern unsigned component_size;
	void init(RectangleRendererComponent& c, Allocator& allocator);
	void deinit(RectangleRendererComponent& c, Allocator& allocator);
	void create(RectangleRendererComponent& c, Entity e, Allocator& allocator);
	void destroy(RectangleRendererComponent& c, Entity e);
	void set_rect(RectangleRendererComponent& c, Entity e, const Rect& rect);
	const Rect& rect(RectangleRendererComponent& c, Entity e);
	void set_color(RectangleRendererComponent& c, Entity e, const Color& color);
	const Color& color(RectangleRendererComponent& c, Entity e);
	void set_render_handle(RectangleRendererComponent& c, Entity e, RenderResourceHandle render_handle);
	RenderResourceHandle material(RectangleRendererComponent& c, Entity e);
	void set_material(RectangleRendererComponent& c, Entity e, RenderResourceHandle material);
	RenderResourceHandle render_handle(RectangleRendererComponent& c, Entity e);
	RectangleRendererComponentData* copy_data(RectangleRendererComponent& c, Entity e, Allocator& allocator);
	RectangleRendererComponentData* copy_dirty_data(RectangleRendererComponent& c, Allocator& allocator);
}

}