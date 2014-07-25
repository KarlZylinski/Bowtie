#include "opengl_renderer.h"
#include <cassert>
#include <foundation/array.h>
#include <foundation/file.h>
#include <foundation/memory.h>
#include <foundation/matrix4.h>
#include <engine/shader_utils.h>
#include <engine/view.h>
#include <engine/renderer_command.h>
#include <engine/render_resource_types.h>
#include <renderer/render_drawable.h>
#include <renderer/render_target.h>
#include <renderer/render_texture.h>
#include <renderer/render_world.h>
#include "gl3w.h"

namespace bowtie
{

////////////////////////////////
// Implementation fordward decl.

namespace
{

struct GLPixelFormat;

GLuint compile_glsl_shader(const char* shader_source, GLenum shader_type);
RenderResourceHandle create_fullscreen_rendering_quad();
RenderTexture* create_texture(Allocator& allocator, image::PixelFormat pf, const Vector2u& resolution, void* data);
RenderTarget* create_render_target_internal(Allocator& allocator, const Vector2u& resolution);
void initialize_gl();
GLPixelFormat gl_pixel_format(image::PixelFormat pixel_format);
GLuint link_glsl_program(const GLuint* shaders, int shader_count, bool delete_shaders);
RenderResourceHandle load_rendered_worlds_combining_shader(Allocator& allocator);
GLuint load_shader_internal(const char* vertex_source, const char* fragment_source);
void draw_drawable(const Matrix4& view_projection, const RenderDrawable& drawable, const RenderResourceLookupTable& resource_lut);

}

////////////////////////////////
// Public interface.

OpenGLRenderer::OpenGLRenderer(Allocator& allocator, RenderResourceLookupTable& resource_lut)
	: _allocator(allocator), _resource_lut(resource_lut)
{
}

OpenGLRenderer::~OpenGLRenderer()
{
}

void OpenGLRenderer::clear()
{
	glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLRenderer::combine_rendered_worlds(const Array<RenderWorld*>& rendered_worlds)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, _resolution.x, _resolution.y);
	clear();
	auto shader = _rendered_worlds_combining_shader.render_handle;
	glUseProgram(shader);

	for (unsigned i = 0; i < array::size(rendered_worlds); ++i)
	{
		auto& rw = *rendered_worlds[i];
		auto& rt = rw.render_target();
		
		if (i == 0)
		{
			GLuint texture_sampler_id = glGetUniformLocation(shader, "texture_sampler1");
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, rt.render_texture.render_handle);
			glUniform1i(texture_sampler_id, 0);
		}

		if (i == 1)
		{
			GLuint texture_sampler_id = glGetUniformLocation(shader, "texture_sampler2");
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, rt.render_texture.render_handle);
			glUniform1i(texture_sampler_id, 1);
		}
	}

	auto geometry = _fullscreen_rendering_quad.render_handle;
		
	glBindBuffer(GL_ARRAY_BUFFER, geometry);
	glEnableVertexAttribArray(0);
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

RenderTarget* OpenGLRenderer::create_render_target()
{
	return create_render_target_internal(_allocator, _resolution);
}

void OpenGLRenderer::draw(const View& view, const RenderWorld& render_world)
{	
	auto view_projection = view.view_projection();
	auto& drawables = render_world.drawables();

	for (unsigned i = 0; i < array::size(drawables); ++i)
		draw_drawable(view_projection, *(RenderDrawable*)drawables[i].render_object, _resource_lut);
}

void OpenGLRenderer::initialize_thread()
{
	initialize_gl();
	_fullscreen_rendering_quad = create_fullscreen_rendering_quad();
	_rendered_worlds_combining_shader = load_rendered_worlds_combining_shader(_allocator);
}

RenderResourceHandle OpenGLRenderer::load_geometry(const GeometryResourceData& geometry_data, void* dynamic_data)
{
	GLuint geometry_buffer;
	glGenBuffers(1, &geometry_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
	glBufferData(GL_ARRAY_BUFFER, geometry_data.size, dynamic_data, GL_STATIC_DRAW);

	return RenderResourceHandle(geometry_buffer);
}

RenderResourceHandle OpenGLRenderer::load_shader(const ShaderResourceData& shader_data, void* dynamic_data)
{
	auto vertex_source = (char*)memory::pointer_add(dynamic_data, shader_data.vertex_shader_source_offset);
	auto fragment_source = (char*)memory::pointer_add(dynamic_data, shader_data.fragment_shader_source_offset);
	return RenderResourceHandle(load_shader_internal(vertex_source, fragment_source));
}

RenderResourceHandle OpenGLRenderer::load_texture(const TextureResourceData& trd, void* dynamic_data)
{
	auto texture = create_texture(_allocator, trd.pixel_format, trd.resolution, memory::pointer_add(dynamic_data, trd.texture_data_dynamic_data_offset));
	return RenderResourceHandle(texture);
}

void OpenGLRenderer::update_geometry(const DrawableGeometryReflectionData& geometry_data, void* dynamic_data)
{
	auto& drawable = *(RenderDrawable*)_resource_lut.lookup(geometry_data.drawable).render_object;
	auto geometry_handle = _resource_lut.lookup(drawable.geometry).render_handle;
	glBindBuffer(GL_ARRAY_BUFFER, geometry_handle);
	glBufferData(GL_ARRAY_BUFFER, geometry_data.size, dynamic_data, GL_STATIC_DRAW);
}

void OpenGLRenderer::resize(const Vector2u& resolution, Array<RenderTarget*>&)
{
	_resolution = resolution;
	glViewport(0, 0, _resolution.x, _resolution.y);
}

const Vector2u& OpenGLRenderer::resolution() const
{
	return _resolution;
}

void OpenGLRenderer::set_render_target(const RenderTarget& render_target)
{
	glBindFramebuffer(GL_FRAMEBUFFER, render_target.target_handle.render_handle);
	glViewport(0, 0, _resolution.x, _resolution.y);
}


////////////////////////////////
// Implementation.

namespace
{

struct GLPixelFormat {
	GLenum format;
	GLenum internal_format;
};

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

RenderTexture* create_texture(Allocator& allocator, image::PixelFormat pf, const Vector2u& resolution, void* data)
{
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	auto pixel_format = gl_pixel_format(pf);
	glTexImage2D(GL_TEXTURE_2D, 0, pixel_format.internal_format, resolution.x, resolution.y, 0, pixel_format.format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	RenderTexture* render_texture = (RenderTexture*)allocator.allocate(sizeof(RenderTexture));
	render_texture->pixel_format = pf;
	render_texture->render_handle = RenderResourceHandle(texture_id);
	render_texture->resolution = resolution;
	return render_texture;
}

RenderTarget* create_render_target_internal(Allocator& allocator, const Vector2u& resolution)
{
	auto texture = create_texture(allocator, image::RGBA, resolution, 0);
	auto texture_id = texture->render_handle.render_handle;
	glBindTexture(GL_TEXTURE_2D, texture_id);
	
	GLuint fb = 0;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_id, 0);
	GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, draw_buffers);

	return MAKE_NEW(allocator, RenderTarget, RenderResourceHandle(texture_id), RenderResourceHandle(fb));
}

void initialize_gl()
{
	int extension_load_error = gl3wInit();
	assert(extension_load_error == 0);
		
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	// 2D needs no depth test.
	glDisable(GL_DEPTH_TEST);
}

RenderResourceHandle create_fullscreen_rendering_quad()
{
	static const GLfloat fullscreen_quad_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f,  1.0f, 0.0f,
	};
 
	GLuint quad_vertexbuffer;
	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(fullscreen_quad_data), fullscreen_quad_data, GL_STATIC_DRAW);
	return RenderResourceHandle(quad_vertexbuffer);
}

RenderResourceHandle load_rendered_worlds_combining_shader(Allocator& allocator)
{
	auto shader_source = file::load("rendered_world_combining.shader", allocator);
	auto split_shader = shader_utils::split_shader(shader_source, allocator);
	auto shader_handle = load_shader_internal(split_shader.vertex_source, split_shader.fragment_source);
	allocator.deallocate(shader_source.data);
	allocator.deallocate(split_shader.vertex_source);
	allocator.deallocate(split_shader.fragment_source);
	return RenderResourceHandle(shader_handle);
}

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

GLuint load_shader_internal(const char* vertex_source, const char* fragment_source)
{
	GLuint vertex_shader = compile_glsl_shader(vertex_source, GL_VERTEX_SHADER);
	GLuint fragment_shader = compile_glsl_shader(fragment_source, GL_FRAGMENT_SHADER);
		
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

void draw_drawable(const Matrix4& view_projection, const RenderDrawable& drawable, const RenderResourceLookupTable& resource_lut)
{
	auto model_view_projection_matrix = drawable.model * view_projection;
		
	auto shader = resource_lut.lookup(drawable.shader).render_handle;
	assert(glIsProgram(shader) && "Invalid shader program");
	glUseProgram(shader);

	GLuint model_view_projection_matrix_id = glGetUniformLocation(shader, "model_view_projection_matrix");
	glUniformMatrix4fv(model_view_projection_matrix_id, 1, GL_FALSE, &model_view_projection_matrix[0][0]);
		
	if (drawable.texture.type != ResourceHandle::NotInitialized)
	{
		RenderTexture& drawable_texture = *(RenderTexture*)resource_lut.lookup(drawable.texture).render_object;
		GLuint texture_sampler_id = glGetUniformLocation(shader, "texture_sampler");
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, drawable_texture.render_handle.render_handle);
		glUniform1i(texture_sampler_id, 0);
	}

	auto geometry = resource_lut.lookup(drawable.geometry).render_handle;
		
	glBindBuffer(GL_ARRAY_BUFFER, geometry);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		9 * sizeof(float),
		(void*)0 
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		9 * sizeof(float),
		(void*)(3 * sizeof(float))
	);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,
		3,
		GL_FLOAT,
		GL_FALSE,
		9 * sizeof(float),
		(void*)(5 * sizeof(float))
	);

	glDrawArrays(GL_TRIANGLES, 0, drawable.num_vertices);
	glDisableVertexAttribArray(0);
}

} // implementation

} // namespace bowtie
