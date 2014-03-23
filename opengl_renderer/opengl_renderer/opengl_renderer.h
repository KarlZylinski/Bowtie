#pragma once

#include <engine/renderer.h>

namespace bowtie
{

class OpenGLRenderer : public Renderer
{
public:
	OpenGLRenderer(Allocator& allocator);
	~OpenGLRenderer();
	
	virtual void test_draw(const View& view);
	virtual void clear();
	virtual void flip();
	virtual void resize(const Vector2u& resolution);

	// Resource loading
	virtual InternalRenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data);

protected:
	virtual void run_thread();
	
private:	
	// Disabled stuff
	OpenGLRenderer(OpenGLRenderer&);
	OpenGLRenderer& operator=(OpenGLRenderer&);
};

}