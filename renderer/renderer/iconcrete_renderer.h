#pragma once

#include <engine/render_resource_handle.h>
#include <foundation/collection_types.h>
#include "render_resource.h"

namespace bowtie
{

struct DrawableGeometryReflectionData;
struct GeometryResourceData;
struct RenderTarget;
struct RenderTexture;
class RenderWorld;
struct ShaderResourceData;
struct TextureResourceData;
class View;
struct Vector2u;

class IConcreteRenderer
{
public:
	virtual ~IConcreteRenderer() {}
	virtual void clear() = 0;
	virtual void combine_rendered_worlds(const Array<RenderWorld*>& rendered_worlds) = 0;
	virtual RenderTarget* create_render_target() = 0;
	virtual void destroy_render_target(const RenderTarget& target) = 0;
	virtual void draw(const View& view, const RenderWorld& render_world) = 0;
	virtual unsigned get_uniform_location(RenderResource shader, const char* name) = 0;
	virtual void initialize_thread() = 0;
	virtual RenderResource load_geometry(const GeometryResourceData& geometry_data, void* dynamic_data) = 0;
	virtual RenderTexture* load_texture(const TextureResourceData& trd, void* dynamic_data) = 0;
	virtual RenderResource load_shader(const ShaderResourceData& shader_data, void* dynamic_data) = 0;
	virtual void update_geometry(const DrawableGeometryReflectionData& geometry_data, void* dynamic_data) = 0;	
	virtual RenderResource update_shader(const RenderResource& shader, const ShaderResourceData& shader_data, void* dynamic_data) = 0;	
	virtual void resize(const Vector2u& size, Array<RenderTarget*>& render_targets) = 0;
	virtual const Vector2u& resolution() const = 0;
	virtual void set_render_target(const RenderTarget& render_target) = 0;
	virtual void unload_geometry(RenderResource handle) = 0;
	virtual void unload_texture(const RenderTexture& texture) = 0;
	virtual void unload_shader(RenderResource handle) = 0;
	virtual void unset_render_target() = 0;
};

}
