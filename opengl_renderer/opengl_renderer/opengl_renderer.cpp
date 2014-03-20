#include "opengl_renderer.h"

#include <cassert>

#include "gl3w.h"

namespace bowtie
{

/////////////////
// Public members

OpenGLRenderer::OpenGLRenderer() : _context(nullptr)
{
}

OpenGLRenderer::~OpenGLRenderer()
{
	_rendering_thread.join();
}

void OpenGLRenderer::run_render_thread()
{
	assert(_context);

	_rendering_thread = std::thread(&OpenGLRenderer::run, this);
}

void OpenGLRenderer::set_opengl_context(OpenGLContext* context)
{
	_context = context;
}

void OpenGLRenderer::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

//////////////////
// Private members

void OpenGLRenderer::run()
{
	_context->make_current_for_calling_thread();

	int extension_load_error = gl3wInit();
	assert(extension_load_error == 0);

	set_active(true);

	while(active())
	{
		clear();
		_context->flip();
	}
}

}
