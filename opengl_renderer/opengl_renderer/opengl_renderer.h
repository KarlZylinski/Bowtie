#pragma once

#include <thread>
#include <engine/renderer.h>

#include "opengl_context.h"

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
	virtual void run_render_thread();
	void set_opengl_context(OpenGLContext* context);

	// Resource loading
	virtual InternalRenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data);
private:
	void run();
	
	OpenGLContext* _context;
	std::thread _rendering_thread;
		
	// Disabled stuff
	OpenGLRenderer(OpenGLRenderer&);
	OpenGLRenderer& operator=(OpenGLRenderer&);
};

}