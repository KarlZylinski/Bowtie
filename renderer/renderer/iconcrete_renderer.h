#pragma once

#include <engine/resource_handle.h>
#include <foundation/collection_types.h>
#include "render_resource_handle.h"

namespace bowtie
{

struct DrawableGeometryReflectionData;
struct GeometryResourceData;
struct RenderTarget;
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
	virtual void draw(const View& view, const RenderWorld& render_world) = 0;
	virtual void initialize_thread() = 0;
	virtual RenderResourceHandle load_texture(TextureResourceData& trd, void* dynamic_data) = 0;
	virtual RenderResourceHandle load_shader(ShaderResourceData& shader_data, void* dynamic_data) = 0;
	virtual RenderResourceHandle load_geometry(GeometryResourceData& geometry_data, void* dynamic_data) = 0;
	virtual void update_geometry(DrawableGeometryReflectionData& geometry_data, void* dynamic_data) = 0;	
	virtual void resize(const Vector2u& size, Array<RenderTarget*>& render_targets) = 0;
	virtual const Vector2u& resolution() const = 0;
	virtual void set_render_target(const RenderTarget& render_target) = 0;
};

}
