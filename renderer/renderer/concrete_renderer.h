#pragma once

#include <engine/render_resource_handle.h>
#include <engine/image.h>
#include <foundation/collection_types.h>
#include "render_resource.h"

namespace bowtie
{

struct DrawableGeometryReflectionData;
struct GeometryResourceData;
struct RenderDrawable;
struct RenderTarget;
struct RenderTexture;
struct RenderWorld;
struct ShaderResourceData;
struct TextureResourceData;
class View;
struct Vector2u;
class RenderResourceLookupTable;

struct ConcreteRenderer
{
	// Initialization
	void (*initialize_thread)();
	
	// Resource management
	RenderResource (*create_render_target)(const RenderTexture& texture);
	void (*destroy_render_target)(RenderResource render_target);
	unsigned (*get_uniform_location)(RenderResource shader, const char* name);
	RenderResource (*create_geometry)(void* data, unsigned data_size);
	void (*destroy_geometry)(RenderResource handle);
	void (*update_geometry)(RenderDrawable& drawable, void* data, unsigned data_size);
	RenderResource (*create_texture)(image::PixelFormat pf, const Vector2u& resolution, void* data);
	void (*destroy_texture)(RenderResource texture);
	RenderResource (*create_shader)(const char* vertex_source, const char* fragment_source);
	void (*destroy_shader)(RenderResource handle);
	RenderResource (*update_shader)(const RenderResource& shader, const char* vertex_source, const char* fragment_source);
		
	// State setters
	void (*resize)(const Vector2u& size, Array<RenderTarget>& render_targets);
	void (*set_render_target)(const Vector2u& resolution, RenderResource render_target);
	void (*unset_render_target)(const Vector2u& resolution);

	// Drawing
	void (*clear)();
	void (*draw)(const View& view, const RenderWorld& render_world, const Vector2u& resolution, const RenderResourceLookupTable& resource_lut);
	void (*combine_rendered_worlds)(RenderResource fullscreen_rendering_quad, RenderResource rendered_worlds_combining_shader, const Array<RenderWorld*>& rendered_worlds);
};

}
