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
#include <engine/timer.h>
#include <renderer/render_material.h>
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

void combine_rendered_world_internal(GLuint fullscreen_rendering_quad, const Array<RenderWorld*>& rendered_worlds, GLuint shader);
GLuint compile_glsl_shader(const char* shader_source, GLenum shader_type);
GLuint create_fullscreen_rendering_quad();
RenderTexture* create_texture(Allocator& allocator, image::PixelFormat pf, const Vector2u& resolution, void* data);
RenderTarget* create_render_target_internal(Allocator& allocator, const Vector2u& resolution);
void draw_drawable(const Vector2u& resolution, const View& view, const Matrix4& view_matrix, const Matrix4& view_projection_matrix, const RenderDrawable& drawable, const RenderResourceLookupTable& lookup_table);
GLPixelFormat gl_pixel_format(image::PixelFormat pixel_format);
void initialize_gl();
GLuint link_glsl_program(const GLuint* shaders, int shader_count, bool delete_shaders);
GLuint load_rendered_worlds_combining_shader(Allocator& allocator);
GLuint load_shader_internal(const char* vertex_source, const char* fragment_source);
void set_render_target_internal(const Vector2u& resolution, GLuint render_target);

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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OpenGLRenderer::combine_rendered_worlds(const Array<RenderWorld*>& rendered_worlds)
{
	combine_rendered_world_internal(_fullscreen_rendering_quad, rendered_worlds, _rendered_worlds_combining_shader);
}

RenderTarget* OpenGLRenderer::create_render_target()
{
	return create_render_target_internal(_allocator, _resolution);
}

void OpenGLRenderer::destroy_render_target(const RenderTarget& target)
{
	unload_texture(*target.render_texture);
	glDeleteFramebuffers(1, &target.target_handle.handle);
}

void OpenGLRenderer::draw(const View& view, const RenderWorld& render_world)
{	
	auto view_matrix = view.view();
	auto view_projection_matrix = view_matrix * view.projection();
	auto& drawables = render_world.drawables();

	for (unsigned i = 0; i < array::size(drawables); ++i)
		draw_drawable(resolution(), view, view_matrix, view_projection_matrix, *drawables[i], _resource_lut);
}

unsigned OpenGLRenderer::get_uniform_location(RenderResource shader, const char* name)
{
	return glGetUniformLocation(shader.handle, name);
}

void OpenGLRenderer::initialize_thread()
{
	initialize_gl();
	_fullscreen_rendering_quad = create_fullscreen_rendering_quad();
	_rendered_worlds_combining_shader = load_rendered_worlds_combining_shader(_allocator);
}

RenderResource OpenGLRenderer::load_geometry(const GeometryResourceData& geometry_data, void* dynamic_data)
{
	GLuint geometry_buffer;
	glGenBuffers(1, &geometry_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
	glBufferData(GL_ARRAY_BUFFER, geometry_data.size, dynamic_data, GL_STATIC_DRAW);

	return RenderResource(geometry_buffer);
}

RenderResource OpenGLRenderer::load_shader(const ShaderResourceData& shader_data, void* dynamic_data)
{
	auto vertex_source = (char*)memory::pointer_add(dynamic_data, shader_data.vertex_shader_source_offset);
	auto fragment_source = (char*)memory::pointer_add(dynamic_data, shader_data.fragment_shader_source_offset);
	return RenderResource(load_shader_internal(vertex_source, fragment_source));
}

RenderTexture* OpenGLRenderer::load_texture(const TextureResourceData& trd, void* dynamic_data)
{
	return create_texture(_allocator, trd.pixel_format, trd.resolution, memory::pointer_add(dynamic_data, trd.texture_data_dynamic_data_offset));
}

void OpenGLRenderer::update_geometry(const DrawableGeometryReflectionData& geometry_data, void* dynamic_data)
{
	auto& drawable = *(RenderDrawable*)_resource_lut.lookup(geometry_data.drawable).object;
	auto geometry_handle = drawable.geometry.handle;
	glBindBuffer(GL_ARRAY_BUFFER, geometry_handle);
	glBufferData(GL_ARRAY_BUFFER, geometry_data.size, dynamic_data, GL_STATIC_DRAW);
}

RenderResource OpenGLRenderer::update_shader(const RenderResource& shader, const ShaderResourceData& shader_data, void* dynamic_data)
{
	auto vertex_source = (char*)memory::pointer_add(dynamic_data, shader_data.vertex_shader_source_offset);
	auto fragment_source = (char*)memory::pointer_add(dynamic_data, shader_data.fragment_shader_source_offset);
	glDeleteProgram(shader.handle);
	return RenderResource(load_shader_internal(vertex_source, fragment_source));	
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
	set_render_target_internal(_resolution, render_target.target_handle.handle);
}

void OpenGLRenderer::unload_geometry(RenderResource handle)
{
	glDeleteBuffers(1, &handle.handle);
}

void OpenGLRenderer::unload_shader(RenderResource handle)
{
	glDeleteProgram(handle.handle);
}

void OpenGLRenderer::unload_texture(const RenderTexture& texture)
{
	glDeleteTextures(1, &texture.render_handle.handle);
}

void OpenGLRenderer::unset_render_target()
{
	set_render_target_internal(_resolution, 0);
}


////////////////////////////////
// Implementation.

namespace
{

struct GLPixelFormat {
	GLenum format;
	GLenum internal_format;
};

void combine_rendered_world_internal(GLuint fullscreen_rendering_quad, const Array<RenderWorld*>& rendered_worlds, GLuint shader)
{
	glUseProgram(shader);
	assert(array::size(rendered_worlds) <= 16);
	GLuint texture_sampler_id = glGetUniformLocation(shader, "texture_samplers");

	for (unsigned i = 0; i < array::size(rendered_worlds); ++i)
	{
		auto& rw = *rendered_worlds[i];
		auto& rt = rw.render_target();
		
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, rt.render_texture->render_handle.handle);
		glUniform1i(texture_sampler_id, i);
	}

	GLuint num_samplers_id = glGetUniformLocation(shader, "num_samplers");
	glUniform1i(num_samplers_id, array::size(rendered_worlds));
			
	glBindBuffer(GL_ARRAY_BUFFER, fullscreen_rendering_quad);
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
	render_texture->render_handle = RenderResource(texture_id);
	render_texture->resolution = resolution;
	return render_texture;
}

RenderTarget* create_render_target_internal(Allocator& allocator, const Vector2u& resolution)
{
	auto texture = create_texture(allocator, image::RGBA, resolution, 0);
	auto texture_id = texture->render_handle.handle;
	glBindTexture(GL_TEXTURE_2D, texture_id);
	
	GLuint fb = 0;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_id, 0);
	GLenum draw_buffers[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, draw_buffers);

	return allocator.construct<RenderTarget>(allocator, texture, RenderResource(fb));
}

GLuint create_fullscreen_rendering_quad()
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
	return quad_vertexbuffer;
}

void draw_drawable(const Vector2u& resolution, const View& view, const Matrix4& view_matrix, const Matrix4& view_projection_matrix, const RenderDrawable& drawable, const RenderResourceLookupTable& lookup_table)
{
	auto model_view_projection_matrix = drawable.model * view_projection_matrix;
	auto model_view_matrix = drawable.model * view_matrix;
	auto& material = *(RenderMaterial*)lookup_table.lookup(drawable.material).object;
	auto shader = lookup_table.lookup(material.shader()).handle;
	assert(glIsProgram(shader) && "Invalid shader program");
	glUseProgram(shader);
	auto time = timer::counter();
	auto view_resolution_ratio = view.rect().size.y / resolution.y;
	auto resoultion_float = Vector2((float)resolution.x, (float)resolution.y);

	auto uniforms = material.uniforms();
	for (unsigned i = 0; i < array::size(uniforms); ++i)
	{
		const auto& uniform = uniforms[i];
		auto value = uniform.value;

		switch (uniform.automatic_value)
		{
		case uniform::ModelViewProjectionMatrix:
			value = (void*)&model_view_projection_matrix[0][0];
			break;
		case uniform::ModelViewMatrix:
			value = (void*)&model_view_matrix[0][0];
			break;
		case uniform::ModelMatrix:
			value = (void*)&drawable.model[0][0];
			break;
		case uniform::Time:
			value = &time;
			break;
		case uniform::DrawableTexture:
			value = (void*)&drawable.texture;
			break;
		case uniform::ViewResolution:
			value = (void*)&view.rect().size;
			break;
		case uniform::ViewResolutionRatio: 
			value = (void*)&view_resolution_ratio;
			break;
		case uniform::Resolution:
			value = (void*)&resoultion_float;
			break;
		}

		switch (uniform.type)
		{
		case uniform::Float: glUniform1fv(uniform.location, 1, (GLfloat*)value); break;
		case uniform::Vec2: glUniform2fv(uniform.location, 1, (GLfloat*)value); break;
		case uniform::Vec3: glUniform3fv(uniform.location, 1, (GLfloat*)value); break;
		case uniform::Vec4: glUniform4fv(uniform.location, 1, (GLfloat*)value); break;
		case uniform::Mat3: glUniformMatrix3fv(uniform.location, 1, GL_FALSE, (GLfloat*)value); break;
		case uniform::Mat4: glUniformMatrix4fv(uniform.location, 1, GL_FALSE, (GLfloat*)value); break;
		case uniform::Texture1:
			{
				glActiveTexture(GL_TEXTURE0);
				auto texture_handle = *(RenderResourceHandle*)value;
				auto texture = *(RenderTexture*)lookup_table.lookup(texture_handle).object;
				glBindTexture(GL_TEXTURE_2D, value == nullptr ? 0 : texture.render_handle.handle);
				glUniform1i(uniform.location, 0);
			} break;
		default:
			assert(!"Unknown uniform type");
		}
	}

	auto geometry = drawable.geometry.handle;
		
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

void initialize_gl()
{
	int extension_load_error = gl3wInit();
	assert(extension_load_error == 0);
		
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
}

GLuint load_rendered_worlds_combining_shader(Allocator& allocator)
{
	auto shader_source = file::load("rendered_world_combining.shader", allocator);
	auto split_shader = shader_utils::split_shader(shader_source, allocator);
	auto shader_handle = load_shader_internal(split_shader.vertex_source, split_shader.fragment_source);
	allocator.deallocate(shader_source.data);
	allocator.deallocate(split_shader.vertex_source);
	allocator.deallocate(split_shader.fragment_source);
	return shader_handle;
}
	
GLuint link_glsl_program(const GLuint* shaders, int shader_count, bool delete_shaders)
{
	int i;
	GLuint program = glCreateProgram();

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

void set_render_target_internal(const Vector2u& resolution, GLuint render_target)
{	
	glBindFramebuffer(GL_FRAMEBUFFER, render_target);
	glViewport(0, 0, resolution.x, resolution.y);
}

} // implementation

} // namespace bowtie
