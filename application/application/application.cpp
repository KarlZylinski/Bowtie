#include <engine/engine.h>
#include <engine/resource_manager.h>
#include <foundation/array.h>
#include <foundation/temp_allocator.h>
#include <opengl_renderer/opengl_context_windows.h>
#include <opengl_renderer/opengl_renderer.h>
#include <os/windows/callstack_capturer.h>
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
	engine::resize(*s_engine, resolution);
}

void key_down_callback(keyboard::Key key)
{
	engine::key_pressed(*s_engine, key);
}

void key_up_callback(keyboard::Key key)
{
	engine::key_released(*s_engine, key);
}

struct Haze
{
	int lax;
	short bulgur;
};

int WINAPI WinMain(__in HINSTANCE instance, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
	auto callstack_capturer = callstack_capturer::create();
	memory_globals::init(callstack_capturer);
	Allocator& allocator = memory_globals::default_allocator();
	s_allocator = &allocator;	
	Allocator* renderer_allocator = memory_globals::new_allocator(callstack_capturer, "renderer allocator");

	{
		ConcreteRenderer opengl_renderer = opengl_renderer::create();
		Renderer renderer(opengl_renderer, *renderer_allocator, allocator);
		s_renderer = &renderer;
		OpenGLContextWindows context;
		s_context = &context;
		auto& render_interface = renderer.render_interface();

		{
			Engine engine;
			engine::init(engine, allocator, render_interface);
			s_engine = &engine;
			auto resolution = Vector2u(1280, 720);
			Window window(instance, resolution, &create_render_context_callback, &window_resized_callback, &key_down_callback, &key_up_callback);		
			while(window.is_open())
			{
				window.dispatch_messages();
				engine::update(engine);
				renderer.deallocate_processed_commands(allocator);
			}
			engine::deinit(engine);
		}
		
		renderer.stop(allocator);
	}

	memory_globals::destroy_allocator(renderer_allocator);
	memory_globals::shutdown();
}
