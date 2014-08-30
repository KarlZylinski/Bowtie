#pragma once

#include <foundation/vector2u.h>
#include <renderer/iconcrete_renderer.h>
#include <renderer/render_resource_lookup_table.h>

namespace bowtie
{

class OpenGLRenderer : public IConcreteRenderer
{
public:
	typedef unsigned GLuint;

	OpenGLRenderer(Allocator& allocator, RenderResourceLookupTable& resource_lut);
	~OpenGLRenderer();
	
	void clear();
	void combine_rendered_worlds(const Array<RenderWorld*>& rendered_worlds);
	RenderTarget* create_render_target();
	void destroy_render_target(const RenderTarget& target);
	void draw(const View& view, const RenderWorld& render_world);
	unsigned get_uniform_location(RenderResource shader, const char* name);
	void initialize_thread();
	RenderResource load_geometry(const GeometryResourceData& geometry_data, void* dynamic_data);
	RenderResource load_shader(const ShaderResourceData& shader_data, void* dynamic_data);	
	RenderTexture* load_texture(const TextureResourceData& trd, void* dynamic_data);
	void update_geometry(const DrawableGeometryReflectionData& geometry_data, void* dynamic_data);
	void resize(const Vector2u& resolution, Array<RenderTarget*>& render_targets);
	const Vector2u& resolution() const;
	void set_render_target(const RenderTarget& render_target);
	void unload_geometry(RenderResource handle);
	void unload_shader(RenderResource handle);
	void unload_texture(const RenderTexture& texture);
	void unset_render_target();

private:
	Allocator& _allocator;
	RenderResourceLookupTable& _resource_lut;
	GLuint _fullscreen_rendering_quad;
	GLuint _rendered_worlds_combining_shader;
	Vector2u _resolution;

	// Disabled stuff
	OpenGLRenderer(OpenGLRenderer&);
	OpenGLRenderer& operator=(OpenGLRenderer&);
};

}