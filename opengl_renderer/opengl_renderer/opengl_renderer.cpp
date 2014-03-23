#include "opengl_renderer.h"

#include <cassert>

#include <foundation/memory.h>
#include <foundation/matrix4.h>

#include "gl3w.h"

static GLuint vertexbuffer;

namespace bowtie
{

OpenGLRenderer::OpenGLRenderer(Allocator& allocator) : Renderer(allocator), _context(nullptr)
{
}

OpenGLRenderer::~OpenGLRenderer()
{
	set_active(false);
	notify_command_queue_populated();
	_rendering_thread.join();
}

GLuint compile_glsl_shader(const char* shader_source, GLenum shader_type)
{
	GLuint result = glCreateShader(shader_type);

	if(!result)
		return result;

	glShaderSource(result, 1, &shader_source, NULL);
	glCompileShader(result);

	GLint status = 0;
	glGetShaderiv(result, GL_COMPILE_STATUS, &status);

	assert(status && "Compilation of shader failed.");
	
	return result;
}
	
GLuint link_glsl_program(const GLuint* shaders, int shader_count, bool delete_shaders)
{
    int i;
	GLuint program;

    program = glCreateProgram();

    for(i = 0; i < shader_count; i++)
        glAttachShader(program, shaders[i]);

    glLinkProgram(program);

	GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);

    assert(status && "Failed linking shader program");

	if(delete_shaders)
	{
		for(i = 0; i < shader_count; i++)
			glDeleteShader(shaders[i]);
	}
	
	assert(glIsProgram(program));
	
	glValidateProgram(program);
	GLint validation_status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &validation_status);
	assert(validation_status && "Failed to validate program");
	
    return program;
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

void OpenGLRenderer::test_draw(const View& view)
{	
	auto projection_matrix = view.view_projection();
		
	GLuint program = lookup_resource_object(1).handle;

	assert(glIsProgram(program) && "Invalid shader program");
	glUseProgram(program);

	GLuint projection_matrix_id = glGetUniformLocation(program, "model_view_projection_matrix");
	glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, &projection_matrix[0][0]);

	glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		(void*)0 
	);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	glDisableVertexAttribArray(0);
}

void OpenGLRenderer::clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLRenderer::flip()
{
	_context->flip();
}

void OpenGLRenderer::resize(const Vector2u& resolution)
{
	glViewport(0, 0, resolution.x, resolution.y);
}

void OpenGLRenderer::run()
{
	_context->make_current_for_calling_thread();

	int extension_load_error = gl3wInit();
	assert(extension_load_error == 0);
		
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// TEST CODE

	static const GLfloat test_vertices[] = {
	   0.0f,  0.0f, 0.0f,
	   640.0f, 0.0f, 0.0f,
	   320.0f,  480.0f, 0.0f,
	};

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(test_vertices), test_vertices, GL_STATIC_DRAW);

	// 2D needs no depth test.
	glDisable(GL_DEPTH_TEST);

	set_active(true);

	while (active())
	{
		{
			std::unique_lock<std::mutex> command_queue_populated_lock(_command_queue_populated_mutex);
			_wait_for_command_queue_populated.wait(command_queue_populated_lock, [&]{return _command_queue_populated;});
			_command_queue_populated = false;
		}

		consume_command_queue();
	}
}

InternalRenderResourceHandle OpenGLRenderer::load_shader(ShaderResourceData& shader_data, void* dynamic_data)
{
	GLuint vertex_shader = compile_glsl_shader((char*)memory::pointer_add(dynamic_data, shader_data.vertex_shader_source_offset), GL_VERTEX_SHADER);
	GLuint fragment_shader = compile_glsl_shader((char*)memory::pointer_add(dynamic_data, shader_data.fragment_shader_source_offset), GL_FRAGMENT_SHADER);
		
	assert(vertex_shader != 0 && "Failed compiling vertex shader");
	assert(fragment_shader != 0 && "Failed compiling fragments shader");

	GLuint shaders[] = { vertex_shader, fragment_shader	};
	
	GLuint program = link_glsl_program(shaders, 2, true);
	
	if (program == 0)
	{
		char buf[1000];
		int len;
		glGetShaderInfoLog(program, 1000, &len, buf);
		printf("%s", buf);
	}

	assert(program != 0 && "Failed to link glsl shader");
	
	return program;
}

}
