#include <stdio.h>

#include <GLFW/glfw3.h>

#include <foundation/array.h>
#include <foundation/temp_allocator.h>
#include <opengl_renderer/opengl_renderer.h>

int main()
{
	using namespace bowtie;

	GLFWwindow* window;
	
	if(!glfwInit())
		return -1;
		
	window = glfwCreateWindow(640, 480, "Bowtie", NULL, NULL);
	
	if(!window)
	{
		glfwTerminate();
		return -1;
	}
	
	opengl_renderer::OpenGLRenderer renderer(*window);

	while(!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}
}
