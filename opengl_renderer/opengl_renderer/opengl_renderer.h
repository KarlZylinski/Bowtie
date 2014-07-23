#pragma once

#include <foundation/vector2u.h>
#include <engine/image.h>
#include <renderer/iconcrete_renderer.h>
#include <renderer/render_resource_lookup_table.h>
#include "gl3w.h"

namespace bowtie
{

struct RenderTexture;
struct RenderDrawable;
struct Matrix4;
struct GLPixelFormat {
	GLenum format;
	GLenum internal_format;
};

class OpenGLRenderer : public IConcreteRenderer
{
public:
	OpenGLRenderer(Allocator& allocator, RenderResourceLookupTable& resource_lut);
	~OpenGLRenderer();
	
	void clear();
	void combine_rendered_worlds(const Array<RenderWorld*>& rendered_worlds);
	RenderTarget* create_render_target();
	void draw(const View& view, const RenderWorld& render_world);
	void initialize_thread();
	RenderResourceHandle load_geometry(GeometryResourceData& geometry_data, void* dynamic_data);
	RenderResourceHandle load_texture(TextureResourceData& trd, void* dynamic_data);
	RenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data);	
	void update_geometry(DrawableGeometryReflectionData& geometry_data, void* dynamic_data);
	void resize(const Vector2u& resolution, Array<RenderTarget*>& render_targets);
	const Vector2u& resolution() const;
	void set_render_target(const RenderTarget& render_target);

private:
	static GLuint compile_glsl_shader(const char* shader_source, GLenum shader_type);
	static RenderResourceHandle create_fullscreen_rendering_quad();
	static RenderTexture* create_texture(Allocator& allocator, image::PixelFormat pf, const Vector2u& resolution, void* data);
	static RenderTarget* create_render_target_internal(Allocator& allocator, const Vector2u& resolution);
	static void initialize_gl();
	static GLPixelFormat gl_pixel_format(image::PixelFormat pixel_format);
	static GLuint link_glsl_program(const GLuint* shaders, int shader_count, bool delete_shaders);
	static RenderResourceHandle load_rendered_worlds_combining_shader(Allocator& allocator);
	static GLuint load_shader_internal(const char* vertex_source, const char* fragment_source);
	static void draw_drawable(const Matrix4& view_projection, const RenderDrawable& drawable, const RenderResourceLookupTable& resource_lut);

	bool _active;
	Allocator& _allocator;
	RenderResourceLookupTable& _resource_lut;
	RenderResourceHandle _fullscreen_rendering_quad;
	RenderResourceHandle _rendered_worlds_combining_shader;
	Vector2u _resolution;

	// Disabled stuff
	OpenGLRenderer(OpenGLRenderer&);
	OpenGLRenderer& operator=(OpenGLRenderer&);
};

}