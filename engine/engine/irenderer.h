#pragma once

#include "render_resource_handle.h"

namespace bowtie
{

class Allocator;
class IRendererContext;
struct RendererCommand;
struct Vector2u;

class IRenderer
{
public:
	virtual ~IRenderer() {};
	virtual void add_renderer_command(const RendererCommand& command) = 0;
	virtual RenderResourceHandle create_handle() = 0;
	virtual void deallocate_processed_commands(Allocator& render_interface_allocator) = 0;
	virtual void free_handle(RenderResourceHandle handle) = 0;
	virtual bool is_active() const = 0;
	virtual bool is_setup() const = 0;
	virtual const Vector2u& resolution() const = 0;
	virtual void run(IRendererContext* context, const Vector2u& resolution) = 0;
	virtual void stop(Allocator& render_interface_allocator) = 0;
};

}
