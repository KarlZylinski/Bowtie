#include <engine/engine.h>
#include <base/malloc_allocator.h>
#include <opengl_renderer/opengl_renderer.h>
#include <engine/renderer/renderer.h>
#include <engine/timer.h>
#include "callstack_capturer.h"
#include "window.h"
#include "opengl_context.h"
#include "timer.h"

using namespace bowtie;

namespace
{
    Engine* s_engine;
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

void* alloc(uint64 size)
{
    return VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void dealloc(void* p)
{
    VirtualFree(p, 0, MEM_RELEASE);
}

DWORD WINAPI renderer_thread_proc(void* param)
{
    auto renderer = (Renderer*)param;    
    renderer::initialize_thread(renderer);

    while (renderer->active)
        renderer::process_command_queue(renderer);

    return 0;
}

int WINAPI WinMain(__in HINSTANCE instance, __in_opt HINSTANCE, __in_opt LPSTR, __in int)
{
    // Alloc memory
    const auto permanent_memory_size = 33554432u; // 32 megabyte
    void* main_thread_memory_buffer = alloc(permanent_memory_size);
    memory::init(&MainThreadMemory, main_thread_memory_buffer, permanent_memory_size);
    void* render_thread_memory_buffer = alloc(permanent_memory_size);
    memory::init(&RenderThreadMemory, render_thread_memory_buffer, permanent_memory_size);
    const auto temp_memory_size = 134217728u; // 128 megabytes
    void* temp_memory_buffer = alloc(temp_memory_size);
    temp_memory::init(temp_memory_buffer, temp_memory_size);
    auto callstack_capturer = windows::callstack_capturer::create();
    auto allocator = new MallocAllocator();
    memory::init_allocator(allocator, "default allocator", &callstack_capturer);
    auto renderer_allocator = new MallocAllocator();
    memory::init_allocator(renderer_allocator, "renederer allocator", &callstack_capturer);

    // Setup renderer
        
    // Setup engine
    Timer timer = {};
    timer.counter = windows::timer::counter;
    timer.start = windows::timer::start;
    auto opengl_renderer = opengl_renderer::create();
    auto renderer_context = windows::opengl_context::create();
    Engine engine = {};
    engine::init(&engine, allocator, &opengl_renderer, &renderer_context, renderer_allocator, &timer);
    s_engine = &engine;

    // Create window
    auto resolution = vector2u::create(1280, 720);
    windows::Window window = {};
    windows::window::init(&window, instance, &resolution, &window_resized_callback, &key_down_callback, &key_up_callback);

    // Setup renderer thread
    PlatformRendererContextData platform_renderer_context_data = {};
    windows::opengl_context::init(&platform_renderer_context_data, window.hwnd);
    renderer::setup(&engine.renderer, &platform_renderer_context_data, &resolution);
    auto render_thread = CreateThread(nullptr, 0, renderer_thread_proc, &engine.renderer, 0, nullptr);
    
    while(window.is_open)
    {
        temp_memory::new_frame();
        windows::window::dispatch_messages(&window);
        engine::update_and_render(&engine);
    }

    engine::deinit(&engine);
    renderer::stop(&engine.renderer);
    WaitForSingleObject(render_thread, INFINITE);
    renderer::deinit(&engine.renderer);

    // Dealloc memory
    memory::deinit_allocator(renderer_allocator);
    memory::deinit_allocator(allocator);
    dealloc(temp_memory_buffer);
    dealloc(main_thread_memory_buffer);
    dealloc(render_thread_memory_buffer);
}
