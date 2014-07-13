#pragma once

#include <renderer/renderer.h>

namespace bowtie
{

class OpenGLRenderer : public Renderer
{
public:
	OpenGLRenderer(Allocator& allocator);
	~OpenGLRenderer();
	
	virtual void draw(const View& view, ResourceHandle render_world);
	virtual void set_render_target(const RenderTarget& render_target);
	virtual void clear();
	virtual void flip();
	virtual void resize(const Vector2u& resolution, Array<RenderTarget*>& render_targets);
	virtual void combine_rendered_worlds(const Array<ResourceHandle>& rendered_worlds);

	// Resource loading
	virtual RenderResourceHandle load_texture(TextureResourceData& trd, void* dynamic_data);
	virtual RenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data);
	virtual RenderResourceHandle load_geometry(GeometryResourceData& geometry_data, void* dynamic_data);
	virtual void update_geometry(DrawableGeometryReflectionData& geometry_data, void* dynamic_data);
	virtual RenderTarget* create_render_target();

protected:
	virtual void run_thread();
	
private:
	RenderResourceHandle _fullscreen_rendering_quad;
	RenderResourceHandle _rendered_worlds_combining_shader;

	// Disabled stuff
	OpenGLRenderer(OpenGLRenderer&);
	OpenGLRenderer& operator=(OpenGLRenderer&);
};

}