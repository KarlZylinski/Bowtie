#include "opengl_renderer.h"
#include <cassert>
#include <foundation/array.h>
#include <foundation/matrix4.h>
#include <engine/view.h>
#include <engine/rect.h>
#include <engine/timer.h>
#include <renderer/render_component.h>
#include <renderer/render_material.h>
#include <renderer/render_drawable.h>
#include <renderer/render_target.h>
#include <renderer/render_texture.h>
#include <renderer/render_world.h>
#include <renderer/render_resource_table.h>
#include "gl3w.h"
#include <foundation/temp_allocator.h>

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

void combine_rendered_worlds(RenderResource fullscreen_rendering_quad, RenderResource rendered_worlds_combining_shader, const Array<RenderWorld*>& rendered_worlds)
{
	auto shader = rendered_worlds_combining_shader.handle;
	auto quad = fullscreen_rendering_quad.handle;

	glUseProgram(shader);
	assert(array::size(rendered_worlds) <= 16);
	GLuint texture_sampler_id = glGetUniformLocation(shader, "texture_samplers");

	for (unsigned i = 0; i < array::size(rendered_worlds); ++i)
	{
		auto& rw = *rendered_worlds[i];
		auto& rt = rw.render_target;

		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, rt.texture.render_handle.handle);
		glUniform1i(texture_sampler_id, i);
	}

	GLuint num_samplers_id = glGetUniformLocation(shader, "num_samplers");
	glUniform1i(num_samplers_id, array::size(rendered_worlds));

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
}

GLuint create_geometry_internal(void* data, unsigned data_size)
{
	GLuint geometry_buffer;
	glGenBuffers(1, &geometry_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, geometry_buffer);
	glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
	return geometry_buffer;
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

void destroy_geometry_internal(GLuint handle)
{
	glDeleteBuffers(1, &handle);
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

void draw_drawable(const Vector2u& resolution, const Rect& view, const Matrix4& view_matrix, const Matrix4& view_projection_matrix, const RenderDrawable& drawable, const RenderResource* resource_table)
{
	auto model_view_projection_matrix = drawable.model * view_projection_matrix;
	auto model_view_matrix = drawable.model * view_matrix;
	auto& material = *(RenderMaterial*)render_resource_table::lookup(resource_table, drawable.material).object;
	auto shader = render_resource_table::lookup(resource_table, material.shader).handle;
	assert(glIsProgram(shader) && "Invalid shader program");
	glUseProgram(shader);
	auto time = timer::counter();
	auto view_resolution_ratio = view.size.y / resolution.y;
	auto resoultion_float = Vector2((float)resolution.x, (float)resolution.y);

	auto uniforms = material.uniforms;
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

void draw_batch(unsigned start, unsigned size, const Array<RenderComponent*>& components, const Vector2u& resolution, const Rect& view, const Matrix4& view_matrix, const Matrix4& view_projection_matrix, const RenderResource* resource_table)
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
			value = (void*)&ident[0][0];
			break;
		case uniform::Time:
			value = &time;
			break;
		case uniform::DrawableTexture:
			value = nullptr;
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

	const unsigned rect_buffer_num_elements = 54;
	const unsigned rect_buffer_size = rect_buffer_num_elements * sizeof(float);
	const unsigned total_buffer_size = rect_buffer_size * size;
	TempAllocator<864000> ta;
	float* buffer = (float*)ta.allocate(total_buffer_size);

	for (unsigned i = start; i < start + size; ++i)
	{
		float* current_buffer = buffer + rect_buffer_num_elements * i;
		auto x = (float)components[i]->rect.position.x;
		auto y = (float)components[i]->rect.position.y;
		auto w = (float)components[i]->rect.size.x;
		auto h = (float)components[i]->rect.size.y;

		auto r = (float)components[i]->color.r;
		auto g = (float)components[i]->color.g;
		auto b = (float)components[i]->color.b;
		auto a = (float)components[i]->color.a;

		float current_buffer_data[rect_buffer_num_elements] = {
			x, y, 0.0f,
			0.0f, 0.0f,
			r, g, b, a,
			x + w, y, 0.0f,
			1.0f, 0.0f,
			r, g, b, a,
			x, y + h, 0.0f,
			0.0f, 1.0f,
			r, g, b, a,

			x + w, y, 0.0f,
			1.0f, 0.0f,
			r, g, b, a,
			x + w, y + h, 0.0f,
			1.0f, 1.0f,
			r, g, b, a,
			x, y + h, 0.0f,
			0.0f, 1.0f,
			r, g, b, a
		};

		memcpy(current_buffer, &current_buffer_data, rect_buffer_size);
	}

	auto geometry = create_geometry_internal(buffer, total_buffer_size);

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

void draw(const Rect& view, const RenderWorld& render_world, const Vector2u& resolution, const RenderResource* resource_table)
{
	if (render_world.components._size == 0)
		return;

	auto view_matrix = view::view_matrix(view);
	auto view_projection_matrix = view_matrix * view::projection_matrix(view);
	unsigned num_components = array::size(render_world.components);
	auto batch_material = render_world.components[0]->material;
	unsigned batch_start = 0;	

	for (unsigned i = 0; i < num_components; ++i)
	{
		auto material = render_world.components[i]->material;

		if (batch_material == material)
			continue;

		draw_batch(batch_start, i - batch_start, render_world.components, resolution, view, view_matrix, view_projection_matrix, resource_table);
		batch_start = i;
		batch_material = material;
	}

	// Draw last batch.
	draw_batch(batch_start, num_components - batch_start, render_world.components, resolution, view, view_matrix, view_projection_matrix, resource_table);

	/*for (unsigned i = 0; i < array::size(render_world.drawables); ++i)
		draw_drawable(resolution, view, view_matrix, view_projection_matrix, *render_world.drawables[i], resource_table);*/
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

void resize(const Vector2u& resolution, Array<RenderTarget>& render_targets)
{
	glViewport(0, 0, resolution.x, resolution.y);

	for (unsigned i = 0; i < array::size(render_targets); ++i)
	{
		auto& rt = render_targets[i];
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

void update_geometry(RenderDrawable& drawable, void* data, unsigned data_size)
{
	auto geometry_handle = drawable.geometry.handle;
	glBindBuffer(GL_ARRAY_BUFFER, geometry_handle);
	glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);
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
	renderer.create_geometry = &create_geometry;
	renderer.create_render_target = &create_render_target;
	renderer.create_shader = &create_shader;
	renderer.create_texture = &create_texture;
	renderer.destroy_geometry = &destroy_geometry;
	renderer.destroy_texture = &destroy_texture;
	renderer.destroy_render_target = &destroy_render_target;
	renderer.destroy_shader = &destroy_shader;
	renderer.draw = &draw;
	renderer.get_uniform_location = &get_uniform_location;
	renderer.initialize_thread = &initialize_thread;
	renderer.resize = &resize;
	renderer.set_render_target = &set_render_target;
	renderer.unset_render_target = &unset_render_target;
	renderer.update_geometry = &update_geometry;
	renderer.update_shader = &update_shader;
	return renderer;
}

} // namespace opengl_renderer

} // namespace bowtie
