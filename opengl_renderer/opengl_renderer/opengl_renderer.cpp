#include "opengl_renderer.h"
#include <cassert>
#include <foundation/array.h>
#include <foundation/matrix4.h>
#include <engine/view.h>
#include <engine/rect.h>
#include <engine/timer.h>
#include <renderer/render_component.h>
#include <renderer/render_material.h>
#include <renderer/render_target.h>
#include <renderer/render_texture.h>
#include <renderer/render_world.h>
#include <renderer/render_resource_table.h>
#include <renderer/constants.h>
#include "gl3w.h"

namespace bowtie
{

namespace
{

struct GLPixelFormat
{
	GLenum format;
	GLenum internal_format;
};

GLPixelFormat gl_pixel_format(PixelFormat pixel_format)
{
	GLPixelFormat gl_pixel_format;
	memset(&gl_pixel_format, 0, sizeof(GLPixelFormat));

	switch (pixel_format)
	{
	case PixelFormat::RGB:
		gl_pixel_format.format = GL_RGB;
		gl_pixel_format.internal_format = GL_RGB;
		break;
	case PixelFormat::RGBA:
		gl_pixel_format.format = GL_RGBA;
		gl_pixel_format.internal_format = GL_RGBA;
		break;
	default: assert(!"Unknown pixel format"); break;
	}

	return gl_pixel_format;
}

void clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

GLuint create_geometry_internal(void* data, unsigned data_size)
{
	GLuint geometry_buffer;
	glGenBuffers(1, &geometry_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
	glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
	return geometry_buffer;
}

void destroy_geometry_internal(GLuint handle)
{
	glDeleteBuffers(1, &handle);
}

void combine_rendered_worlds(RenderResource rendered_worlds_combining_shader, RenderWorld** rendered_worlds, unsigned num_rendered_worlds)
{
	auto shader = rendered_worlds_combining_shader.handle;

	static const float fullscreen_quad_data[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		-1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
	};

	auto quad = create_geometry_internal((void*)fullscreen_quad_data, sizeof(fullscreen_quad_data));
	glUseProgram(shader);
	assert(num_rendered_worlds <= renderer::max_rendered_worlds);
	GLuint texture_sampler_id = glGetUniformLocation(shader, "texture_samplers");

	for (unsigned i = 0; i < num_rendered_worlds; ++i)
	{
		auto& rw = *rendered_worlds[i];
		auto& rt = rw.render_target;

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, rt.texture.render_handle.handle);
		glUniform1i(texture_sampler_id, i);
	}

	GLuint num_samplers_id = glGetUniformLocation(shader, "num_samplers");
	glUniform1i(num_samplers_id, num_rendered_worlds);

	glBindBuffer(GL_ARRAY_BUFFER, quad);
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
	destroy_geometry_internal(quad);
}

RenderResource create_geometry(void* data, unsigned data_size)
{
	return RenderResource(create_geometry_internal(data, data_size));
}

GLuint create_render_target_internal(GLuint texture_id)
{
	glBindTexture(GL_TEXTURE_2D, texture_id);
	GLuint fb = 0;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture_id, 0);
	GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, draw_buffers);
	return fb;
}

RenderResource create_render_target(const RenderTexture& texture)
{
	return RenderResource(create_render_target_internal(texture.render_handle.handle));
}

GLuint compile_glsl(const char* shader_source, GLenum shader_type)
{
	GLuint result = glCreateShader(shader_type);

	if (!result)
		return result;

	glShaderSource(result, 1, &shader_source, NULL);
	glCompileShader(result);
	GLint status = 0;
	glGetShaderiv(result, GL_COMPILE_STATUS, &status);
	assert(status && "Compilation of shader failed.");
	return result;
}

GLuint link_glsl(const GLuint* shaders, int shader_count, bool delete_shaders)
{
	int i;
	GLuint program = glCreateProgram();

	for (i = 0; i < shader_count; i++)
		glAttachShader(program, shaders[i]);

	glLinkProgram(program);
	GLint status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	assert(status && "Failed linking shader program");

	if (delete_shaders)
	{
		for (i = 0; i < shader_count; i++)
			glDeleteShader(shaders[i]);
	}

	assert(glIsProgram(program));
	glValidateProgram(program);
	GLint validation_status;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &validation_status);
	assert(validation_status && "Failed to validate program");

	return program;
}

RenderResource create_shader(const char* vertex_source, const char* fragment_source)
{
	GLuint vertex_shader = compile_glsl(vertex_source, GL_VERTEX_SHADER);
	GLuint fragment_shader = compile_glsl(fragment_source, GL_FRAGMENT_SHADER);
	assert(vertex_shader != 0 && "Failed compiling vertex shader");
	assert(fragment_shader != 0 && "Failed compiling fragments shader");
	GLuint shaders[] = { vertex_shader, fragment_shader };
	GLuint program = link_glsl(shaders, 2, true);

	if (program == 0)
	{
		char buf[1000];
		int len;
		glGetShaderInfoLog(program, 1000, &len, buf);
		printf("%s", buf);
	}

	assert(program != 0 && "Failed to link glsl shader");

	return RenderResource(program);
}

GLuint create_texture_internal(PixelFormat pf, const Vector2u& resolution, void* data)
{
	GLuint texture_id;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	auto pixel_format = gl_pixel_format(pf);
	glTexImage2D(GL_TEXTURE_2D, 0, pixel_format.internal_format, resolution.x, resolution.y, 0, pixel_format.format, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return texture_id;
}

RenderResource create_texture(PixelFormat pf, const Vector2u& resolution, void* data)
{
	return RenderResource(create_texture_internal(pf, resolution, data));
}

void destroy_geometry(RenderResource handle)
{
	destroy_geometry_internal(handle.handle);
}

void destroy_texture(RenderResource texture)
{
	auto rt = (RenderTexture*)texture.object;
	glDeleteTextures(1, &rt->render_handle.handle);
}

void destroy_render_target_internal(const RenderTarget& render_target)
{
	glDeleteTextures(1, &render_target.texture.render_handle.handle);
	glDeleteFramebuffers(1, &render_target.handle.handle);
}

void destroy_render_target(RenderResource render_target)
{
	destroy_render_target_internal(*(RenderTarget*)render_target.object);
}


void draw_batch(Allocator&, unsigned start, unsigned size, const Array<RenderComponent*>& components, const Vector2u& resolution, const Rect& view, const Matrix4& view_matrix, const Matrix4& view_projection_matrix, const RenderResource* resource_table)
{
	auto model_view_projection_matrix = view_projection_matrix;
	auto model_view_matrix = view_matrix;
	auto& material = *(RenderMaterial*)render_resource_table::lookup(resource_table, components[start]->material).object;
	auto shader = render_resource_table::lookup(resource_table, material.shader).handle;
	assert(glIsProgram(shader) && "Invalid shader program");
	glUseProgram(shader);
	auto time = timer::counter();
	auto view_resolution_ratio = view.size.y / resolution.y;
	auto resoultion_float = Vector2((float)resolution.x, (float)resolution.y);
	Matrix4 ident;

	auto uniforms = material.uniforms;
	for (unsigned i = 0; i < material.num_uniforms; ++i)
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
			value = (void*)&ident[0][0];
			break;
		case uniform::Time:
			value = &time;
			break;
		case uniform::ViewResolution:
			value = (void*)&view.size;
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
			if (uniform.location == -1)
				break;

			glActiveTexture(GL_TEXTURE0);
			auto texture_handle = *(RenderResourceHandle*)value;
			auto texture = *(RenderTexture*)render_resource_table::lookup(resource_table, texture_handle).object;
			glBindTexture(GL_TEXTURE_2D, value == nullptr ? 0 : texture.render_handle.handle);
			glUniform1i(uniform.location, 0);
		} break;
		default:
			assert(!"Unknown uniform type");
		}
	}

	static const unsigned draw_buffer_size = 864000;
	static float draw_buffer[draw_buffer_size];
	const unsigned rect_buffer_num_elements = 54;
	const unsigned rect_buffer_size = rect_buffer_num_elements * sizeof(float);
	const unsigned total_buffer_size = rect_buffer_size * size;
	assert(total_buffer_size <= draw_buffer_size && "Draw buffer size exceeded limit. Increase in opengl_renderer.cpp");

	for (unsigned i = start; i < start + size; ++i)
	{
		float* current_buffer = draw_buffer + rect_buffer_num_elements * (i - start);
		const auto& v1 = components[i]->geometry.v1;
		const auto& v2 = components[i]->geometry.v2;
		const auto& v3 = components[i]->geometry.v3;
		const auto& v4 = components[i]->geometry.v4;

		auto r = (float)components[i]->color.r;
		auto g = (float)components[i]->color.g;
		auto b = (float)components[i]->color.b;
		auto a = (float)components[i]->color.a;

		float current_buffer_data[rect_buffer_num_elements] = {
			v1.x, v1.y, 0.0f,
			0.0f, 0.0f,
			r, g, b, a,
			v2.x, v2.y, 0.0f,
			1.0f, 0.0f,
			r, g, b, a,
			v3.x, v3.y, 0.0f,
			0.0f, 1.0f,
			r, g, b, a,

			v2.x, v2.y, 0.0f,
			1.0f, 0.0f,
			r, g, b, a,
			v4.x, v4.y, 0.0f,
			1.0f, 1.0f,
			r, g, b, a,
			v3.x, v3.y, 0.0f,
			0.0f, 1.0f,
			r, g, b, a
		};

		memcpy(current_buffer, &current_buffer_data, rect_buffer_size);
	}

	auto geometry = create_geometry_internal(draw_buffer, total_buffer_size);

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

	glDrawArrays(GL_TRIANGLES, 0, 6 * size);
	glDisableVertexAttribArray(0);

	destroy_geometry_internal(geometry);
}

void draw(Allocator& ta, const Rect& view, const RenderWorld& render_world, const Vector2u& resolution, const RenderResource* resource_table)
{
	if (render_world.components._size == 0)
		return;

	auto view_matrix = view::view_matrix(view);
	auto view_projection_matrix = view_matrix * view::projection_matrix(view);
	unsigned num_components = array::size(render_world.components);
	auto batch_material = render_world.components[0]->material;
	auto batch_depth = render_world.components[0]->depth;
	unsigned batch_start = 0;	

	for (unsigned i = 0; i < num_components; ++i)
	{
		auto material = render_world.components[i]->material;
		auto depth = render_world.components[i]->depth;

		if (batch_material == material && batch_depth == depth)
			continue;

		draw_batch(ta, batch_start, i - batch_start, render_world.components, resolution, view, view_matrix, view_projection_matrix, resource_table);
		batch_start = i;
		batch_material = material;
		batch_depth = depth;
	}

	// Draw last batch.
	draw_batch(ta, batch_start, num_components - batch_start, render_world.components, resolution, view, view_matrix, view_projection_matrix, resource_table);
}

unsigned get_uniform_location(RenderResource shader, const char* name)
{
	return glGetUniformLocation(shader.handle, name);
}

void initialize_thread()
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

void resize(const Vector2u& resolution, RenderTarget* render_targets)
{
	glViewport(0, 0, resolution.x, resolution.y);

	for (unsigned i = 0; i < renderer::max_render_targets; ++i)
	{
		auto& rt = render_targets[i];

		if (rt.handle.type == RenderResource::NotInitialized)
			continue;

		destroy_render_target_internal(rt);
		auto texture = create_texture_internal(rt.texture.pixel_format, resolution, 0);
		auto new_rt = create_render_target_internal(texture);
		rt.handle = RenderResource(new_rt);
		rt.texture.render_handle = RenderResource(texture);
	}
}

void set_render_target(const Vector2u& resolution, RenderResource render_target)
{
	glBindFramebuffer(GL_FRAMEBUFFER, render_target.handle);
	glViewport(0, 0, resolution.x, resolution.y);
}

void destroy_shader(RenderResource handle)
{
	glDeleteProgram(handle.handle);
}

void unset_render_target(const Vector2u& resolution)
{
	set_render_target(resolution, RenderResource(0u));
}

RenderResource update_shader(const RenderResource& shader, const char* vertex_source, const char* fragment_source)
{
	glDeleteProgram(shader.handle);
	return RenderResource(create_shader(vertex_source, fragment_source));
}

}

namespace opengl_renderer
{

ConcreteRenderer create()
{
	ConcreteRenderer renderer;
	renderer.clear = &clear;
	renderer.combine_rendered_worlds = &combine_rendered_worlds;
	renderer.create_render_target = &create_render_target;
	renderer.create_shader = &create_shader;
	renderer.create_texture = &create_texture;
	renderer.destroy_texture = &destroy_texture;
	renderer.destroy_render_target = &destroy_render_target;
	renderer.destroy_shader = &destroy_shader;
	renderer.draw = &draw;
	renderer.get_uniform_location = &get_uniform_location;
	renderer.initialize_thread = &initialize_thread;
	renderer.resize = &resize;
	renderer.set_render_target = &set_render_target;
	renderer.unset_render_target = &unset_render_target;
	renderer.update_shader = &update_shader;
	return renderer;
}

} // namespace opengl_renderer

} // namespace bowtie
