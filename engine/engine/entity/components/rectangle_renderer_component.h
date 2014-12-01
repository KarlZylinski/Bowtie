#pragma once

#include "../../render_resource_handle.h"
#include "component_header.h"

namespace bowtie
{

class Allocator;
class RenderInterface;
struct Material;
struct Rect;
struct Vector4;
struct Matrix4;
struct Quad;
typedef Vector4 Color;

struct RectangleRendererComponentData
{
	Entity* entity;
	Color* color;
	Rect* rect;
	Material* material;
	RenderResourceHandle* render_handle;
	Quad* geometry;
};

struct RectangleRendererComponent
{
	ComponentHeader header;
	void* buffer;
	RectangleRendererComponentData data;
};

namespace rectangle_renderer_component
{
	extern unsigned component_size;
	void init(RectangleRendererComponent& c, Allocator& allocator);
	void deinit(RectangleRendererComponent& c, Allocator& allocator);
	void create(RectangleRendererComponent& c, Entity e, Allocator& allocator, const Rect& rect, const Color& color);
	void destroy(RectangleRendererComponent& c, Entity e);
	void set_rect(RectangleRendererComponent& c, Entity e, const Rect& rect);
	const Rect& rect(RectangleRendererComponent& c, Entity e);
	void set_color(RectangleRendererComponent& c, Entity e, const Color& color);
	const Color& color(RectangleRendererComponent& c, Entity e);
	void set_render_handle(RectangleRendererComponent& c, Entity e, RenderResourceHandle render_handle);
	const Material& material(RectangleRendererComponent& c, Entity e);
	void set_material(RectangleRendererComponent& c, Entity e, Material& material);
	RenderResourceHandle render_handle(RectangleRendererComponent& c, Entity e);
	void set_geometry(RectangleRendererComponent& c, Entity e, const Quad& geometry);
	const Quad& geometry(RectangleRendererComponent& c, Entity e);
	void* copy_dirty_data(RectangleRendererComponent& c, Allocator& allocator);
	void* copy_new_data(RectangleRendererComponent& c, Allocator& allocator);
	RectangleRendererComponentData create_data_from_buffer(void* buffer, unsigned num);
}

}
