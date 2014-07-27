#include <stdio.h>

#include <engine/engine.h>
#include <engine/resource_manager.h>
#include <foundation/array.h>
#include <foundation/temp_allocator.h>
#include <opengl_renderer/opengl_context_windows.h>
#include <opengl_renderer/opengl_renderer.h>
#include <os/windows/window.h>
#include <renderer/renderer.h>

using namespace bowtie;

namespace
{
	Engine* s_engine;
	Renderer* s_renderer;
	OpenGLContextWindows* s_context;
	Allocator* s_allocator;
}

void create_render_context_callback(HWND hwnd, const Vector2u& resolution)
{
	s_context->create(hwnd);
	s_renderer->run(s_context, resolution);
}

void window_resized_callback(const Vector2u& resolution)
{
	s_engine->resize(resolution);
}

void key_down_callback(keyboard::Key key)
{
	s_engine->key_pressed(key);
}

void key_up_callback(keyboard::Key key)
{
	s_engine->key_released(key);
}

int WINAPI WinMain(__in HINSTANCE instance, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
	memory_globals::init();
	Allocator& allocator = memory_globals::default_allocator();
	s_allocator = &allocator;
	
	Allocator* renderer_allocator = memory_globals::new_allocator("renderer allocator");
	{
		RenderResourceLookupTable render_resource_lookup_table;

		OpenGLRenderer opengl_renderer(*renderer_allocator, render_resource_lookup_table);
		Renderer renderer(opengl_renderer, *renderer_allocator, allocator, render_resource_lookup_table);
		s_renderer = &renderer;

		OpenGLContextWindows context;
		s_context = &context;

		Engine engine(allocator, renderer.render_interface());
		s_engine = &engine;

		auto resolution = Vector2u(1280, 720);
		Window window(instance, resolution, &create_render_context_callback, &window_resized_callback,
			&key_down_callback, &key_up_callback);
		
		while(window.is_open())
		{
			window.dispatch_messages();
			engine.update();
		}
	}
	memory_globals::destroy_allocator(renderer_allocator);
	memory_globals::shutdown();
}
