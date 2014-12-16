#include <engine/engine.h>
#include <base/malloc_allocator.h>
#include <opengl_renderer/opengl_renderer.h>
#include <renderer/renderer.h>
#include <engine/timer.h>
#include "callstack_capturer.h"
#include "window.h"
#include "opengl_context.h"
#include "timer.h"

using namespace bowtie;

namespace
{
    Engine* s_engine;
    Renderer* s_renderer;
    PlatformRendererContextData* s_render_context_data;
    Allocator* s_allocator;
}

void create_render_context_callback(HWND hwnd, const Vector2u* resolution)
{
    windows::opengl_context::init(s_render_context_data, hwnd);
    renderer::run(s_renderer, s_render_context_data, resolution);
}

void window_resized_callback(const Vector2u* resolution)
{
    engine::resize(s_engine, resolution);
}

void key_down_callback(Key key)
{
    engine::key_pressed(s_engine, key);
}

void key_up_callback(Key key)
{
    engine::key_released(s_engine, key);
}

PermanentMemory bowtie::MainThreadMemory;
PermanentMemory bowtie::RenderThreadMemory;

int WINAPI WinMain(__in HINSTANCE instance, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
    const auto permanent_memory_size = 33554432u; // 32 megabyte
    void* main_thread_memory_buffer = VirtualAlloc(0, permanent_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    memory::init(&MainThreadMemory, main_thread_memory_buffer, permanent_memory_size);
    void* render_thread_memory_buffer = VirtualAlloc(0, permanent_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    memory::init(&RenderThreadMemory, render_thread_memory_buffer, permanent_memory_size);
    const auto temp_memory_size = 134217728u; // 128 megabytes
    void* temp_memory_buffer = VirtualAlloc(0, temp_memory_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    temp_memory::init(temp_memory_buffer, temp_memory_size);
    auto callstack_capturer = windows::callstack_capturer::create();
    auto allocator = new MallocAllocator();
    memory::init_allocator(allocator, "default allocator", &callstack_capturer);
    s_allocator = allocator;
    auto renderer_allocator = new MallocAllocator();
    memory::init_allocator(renderer_allocator, "renederer allocator", &callstack_capturer);

    {
        ConcreteRenderer opengl_renderer = opengl_renderer::create();
        Renderer renderer;
        auto renderer_context = windows::opengl_context::create();
        renderer::init(&renderer, &opengl_renderer, renderer_allocator, &renderer_context);
        PlatformRendererContextData platform_renderer_context_data = {};
        s_render_context_data = &platform_renderer_context_data;
        s_renderer = &renderer;

        {
            Timer timer = {};
            timer.counter = windows::timer::counter;
            timer.start = windows::timer::start;
            Engine engine = {};
            engine::init(&engine, allocator, &renderer.render_interface, &timer);
            s_engine = &engine;
            auto resolution = vector2u::create(1280, 720);
            windows::Window window = {};
            windows::window::init(&window, instance, &resolution, &create_render_context_callback, &window_resized_callback, &key_down_callback, &key_up_callback);    
                
            while(window.is_open)
            {
                temp_memory::new_frame();
                windows::window::dispatch_messages(&window);
                engine::update_and_render(&engine);
            }

            engine::deinit(&engine);
        }
        
        renderer::stop(&renderer);
        renderer::deinit(&renderer);
    }

    memory::deinit_allocator(renderer_allocator);
    memory::deinit_allocator(allocator);
    VirtualFree(temp_memory_buffer, 0, MEM_RELEASE);
    VirtualFree(main_thread_memory_buffer, 0, MEM_RELEASE);
    VirtualFree(render_thread_memory_buffer, 0, MEM_RELEASE);
}
