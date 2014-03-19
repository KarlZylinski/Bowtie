#include "opengl_renderer.h"
#include <cassert>

namespace bowtie
{

namespace opengl_renderer
{

/////////////////
// Public members

OpenGLRenderer::OpenGLRenderer(GLFWwindow& window) : _window(window)
{
	run_render_thread();
}

OpenGLRenderer::~OpenGLRenderer()
{
	_rendering_thread.join();
}

//////////////////
// Private members

void OpenGLRenderer::run_render_thread()
{
	_rendering_thread = std::thread(&OpenGLRenderer::run, this);
}

void OpenGLRenderer::run()
{
	glfwMakeContextCurrent(&_window);
	
	while(!glfwWindowShouldClose(&_window))
	{
		glfwSwapBuffers(&_window);
	}

	glfwTerminate();
}

}

}
