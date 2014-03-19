#pragma once

#include <GLFW/glfw3.h>
#include <thread>

namespace bowtie
{

namespace opengl_renderer
{

class OpenGLRenderer
{
public:
	OpenGLRenderer(GLFWwindow& window);
	~OpenGLRenderer();

private:
	void run_render_thread();
	void run();

	GLFWwindow& _window;
	std::thread _rendering_thread;
		
	// Disabled stuff
	OpenGLRenderer(OpenGLRenderer&);
	OpenGLRenderer& operator=(OpenGLRenderer&);
};

}

}