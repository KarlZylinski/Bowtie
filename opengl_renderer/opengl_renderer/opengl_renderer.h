#pragma once

#include <engine/renderer.h>

namespace bowtie
{

class OpenGLRenderer : public Renderer
{
public:
	OpenGLRenderer(Allocator& allocator);
	~OpenGLRenderer();
	
	virtual void test_draw(const View& view, ResourceHandle test_sprite);
	virtual void clear();
	virtual void flip();
	virtual void resize(const Vector2u& resolution);
	virtual RenderResourceHandle set_up_sprite_rendering_quad();

	// Resource loading
	virtual RenderResourceHandle load_BMP(TextureResourceData& trd, void* dynamic_data);
	virtual RenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data);

protected:
	virtual void run_thread();
	
private:	
	// Disabled stuff
	OpenGLRenderer(OpenGLRenderer&);
	OpenGLRenderer& operator=(OpenGLRenderer&);
};

}