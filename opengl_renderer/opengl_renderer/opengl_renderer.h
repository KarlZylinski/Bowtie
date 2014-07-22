#pragma once

#include <foundation/vector2u.h>
#include <renderer/iconcrete_renderer.h>
#include <renderer/render_resource_lookup_table.h>

namespace bowtie
{

class OpenGLRenderer : public IConcreteRenderer
{
public:
	OpenGLRenderer(Allocator& allocator, RenderResourceLookupTable& render_resource_lookup_table);
	~OpenGLRenderer();
	
	void clear();
	void combine_rendered_worlds(const Array<ResourceHandle>& rendered_worlds);
	RenderTarget* create_render_target();
	void draw(const View& view, ResourceHandle render_world);
	void initialize_thread();
	bool is_active() const;
	RenderResourceHandle load_texture(TextureResourceData& trd, void* dynamic_data);
	RenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data);
	RenderResourceHandle load_geometry(GeometryResourceData& geometry_data, void* dynamic_data);
	void update_geometry(DrawableGeometryReflectionData& geometry_data, void* dynamic_data);
	void resize(const Vector2u& resolution, Array<RenderTarget*>& render_targets);
	const Vector2u& resolution() const;
	void set_render_target(const RenderTarget& render_target);

private:
	RenderResourceHandle lookup_resource(ResourceHandle handle) const;

	bool _active;
	Allocator& _allocator;
	RenderResourceLookupTable& _render_resource_lookup_table;
	RenderResourceHandle _fullscreen_rendering_quad;
	RenderResourceHandle _rendered_worlds_combining_shader;
	Vector2u _resolution;

	// Disabled stuff
	OpenGLRenderer(OpenGLRenderer&);
	OpenGLRenderer& operator=(OpenGLRenderer&);
};

}