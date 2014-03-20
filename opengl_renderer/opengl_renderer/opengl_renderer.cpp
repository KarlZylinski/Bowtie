#include "opengl_renderer.h"

#include <cassert>

#include "gl3w.h"

namespace bowtie
{

/////////////////
// Public members

OpenGLRenderer::OpenGLRenderer(Allocator& allocator) : Renderer(allocator), _context(nullptr)
{
}

OpenGLRenderer::~OpenGLRenderer()
{
	set_active(false);
	notify_command_queue_populated();
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

void OpenGLRenderer::flip()
{
	_context->flip();
}

//////////////////
// Private members

void OpenGLRenderer::run()
{
	_context->make_current_for_calling_thread();

	int extension_load_error = gl3wInit();
	assert(extension_load_error == 0);
		
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	set_active(true);

	while(active())
	{
		{
			std::unique_lock<std::mutex> command_queue_populated_lock(_command_queue_populated_mutex);
			_wait_for_command_queue_populated.wait(command_queue_populated_lock, [&]{return _command_queue_populated;});
			_command_queue_populated = false;
		}

		consume_command_queue();
	}
}

}
