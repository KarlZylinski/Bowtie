#pragma once

#include <engine/renderer.h>

namespace bowtie
{

class OpenGLRenderer : public Renderer
{
public:
	OpenGLRenderer(Allocator& allocator);
	~OpenGLRenderer();
	
	virtual void draw(const View& view, ResourceHandle render_world);
	virtual void clear();
	virtual void flip();
	virtual void resize(const Vector2u& resolution);

	// Resource loading
	virtual RenderResourceHandle load_texture(TextureResourceData& trd, void* dynamic_data);
	virtual RenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data);
	virtual RenderResourceHandle load_geometry(GeometryResourceData& geometry_data, void* dynamic_data);
	virtual void update_geometry(DrawableGeometryReflectionData& geometry_data, void* dynamic_data);

protected:
	virtual void run_thread();
	
private:	
	// Disabled stuff
	OpenGLRenderer(OpenGLRenderer&);
	OpenGLRenderer& operator=(OpenGLRenderer&);
};

}