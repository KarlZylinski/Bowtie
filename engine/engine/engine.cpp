#include "engine.h"

#include <cstring>

#include "render_interface.h"
#include "renderer_command.h"

namespace bowtie
{

Engine::Engine(Allocator& allocator, RenderInterface& render_interface) : _allocator(allocator), _render_interface(render_interface)
{
}

void Engine::update()
{
	if(!_render_interface.active()) return;

	_render_interface.wait_for_fence(_render_interface.create_fence());
	
	RendererCommand rc;
	memset(&rc, 0, sizeof(RendererCommand));
	rc.type = RendererCommand::RenderWorld;

	_render_interface.dispatch(rc);
}

}