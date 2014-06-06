#include "opengl_renderer.h"

#include <cassert>

#include <engine/render_sprite.h>
#include <engine/render_texture.h>
#include <engine/render_world.h>

#include <foundation/array.h>
#include <foundation/memory.h>
#include <foundation/matrix4.h>

#include "gl3w.h"

namespace bowtie
{

OpenGLRenderer::OpenGLRenderer(Allocator& allocator) : Renderer(allocator)
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

void OpenGLRenderer::test_draw(const View& view, ResourceHandle render_world_handle)
{	
	auto view_projection = view.view_projection();
		
	GLuint program = lookup_resource_object(1).render_handle;

	RenderWorld& render_world = *(RenderWorld*)lookup_resource_object(render_world_handle.handle).render_object;

	auto& sprites = render_world.sprites();

	for (unsigned i = 0; i < array::size(sprites); ++i)
	{
		RenderSprite& sprite = *(RenderSprite*)sprites[i].render_object;
		RenderTexture* sprite_texture = (RenderTexture*)lookup_resource_object(sprite.texture.handle).render_object;

		auto test_image_scale_matrix = Matrix4();
	
		test_image_scale_matrix[0][0] = float(sprite_texture->resolution.x);
		test_image_scale_matrix[1][1] = float(sprite_texture->resolution.y);

		auto model_matrix = test_image_scale_matrix * sprite.model;
		auto model_view_projection_matrix = model_matrix * view_projection;

		assert(glIsProgram(program) && "Invalid shader program");
		glUseProgram(program);

		GLuint model_view_projection_matrix_id = glGetUniformLocation(program, "model_view_projection_matrix");
		glUniformMatrix4fv(model_view_projection_matrix_id, 1, GL_FALSE, &model_view_projection_matrix[0][0]);
		
		GLuint texture_sampler_id = glGetUniformLocation(program, "texture_sampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sprite_texture->render_handle.handle);
		glUniform1i(texture_sampler_id, 0);

		auto sprite_rendering_quad = _resource_lut[_sprite_rendering_quad_handle.handle].render_handle;

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, sprite_rendering_quad);
		glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0 
		);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);
	}
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

RenderResourceHandle OpenGLRenderer::set_up_sprite_rendering_quad()
{
	static const GLfloat sprite_rendering_quad_vertices[] = {
	   0.0f, 0.0f, 0.0f,
	   1.0f, 0.0f, 0.0f,
	   0.0f, 1.0f, 0.0f,

	   1.0f, 0.0f, 0.0f,
	   1.0f, 1.0f, 0.0f,
	   0.0f, 1.0f, 0.0f
	};
	
	static GLuint sprite_rendering_quad_buffer;
	glGenBuffers(1, &sprite_rendering_quad_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sprite_rendering_quad_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sprite_rendering_quad_vertices), sprite_rendering_quad_vertices, GL_STATIC_DRAW);

	return RenderResourceHandle(sprite_rendering_quad_buffer);
}

void OpenGLRenderer::run_thread()
{
	_context->make_current_for_calling_thread();

	int extension_load_error = gl3wInit();
	assert(extension_load_error == 0);
		
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
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

RenderResourceHandle OpenGLRenderer::load_shader(ShaderResourceData& shader_data, void* dynamic_data)
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

struct GLPixelFormat {
	GLenum format;
	GLenum internal_format;
};

GLPixelFormat gl_pixel_format(image::PixelFormat pixel_format)
{
	GLPixelFormat gl_pixel_format;
	memset(&gl_pixel_format, 0, sizeof(GLPixelFormat));

	switch(pixel_format)
	{
	case image::RGB:
		gl_pixel_format.format = GL_RGB;
		gl_pixel_format.internal_format = GL_RGB;
		break;
	case image::RGBA:
		gl_pixel_format.format = GL_RGBA;
		gl_pixel_format.internal_format = GL_RGBA;
		break;
	default: assert(!"Unknown pixel format"); break;
	}

	return gl_pixel_format;
}

RenderResourceHandle OpenGLRenderer::load_texture(TextureResourceData& trd, void* dynamic_data)
{
	GLuint texture_id;
	glGenTextures(1, &texture_id);

	glBindTexture(GL_TEXTURE_2D, texture_id);

	auto pixel_format = gl_pixel_format(trd.pixel_format);

	glTexImage2D(GL_TEXTURE_2D, 0, pixel_format.internal_format, trd.resolution.x, trd.resolution.y, 0, pixel_format.format, GL_UNSIGNED_BYTE, memory::pointer_add(dynamic_data, trd.texture_data_dynamic_data_offset));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	
	RenderTexture* render_texture = (RenderTexture*)_allocator.allocate(sizeof(RenderTexture));
	render_texture->pixel_format = trd.pixel_format;
	render_texture->render_handle = texture_id;
	render_texture->resolution = trd.resolution;

	return render_texture;
}

}
