#include "callstack_capturer.cpp"
#include "opengl_context.cpp"
#include "timer.cpp"
#include "window.cpp"
#include "../base/allocator_helpers.cpp"
#include "../base/concurrent_ring_buffer.cpp"
#include "../base/file.cpp"
#include "../base/jzon.c"
#include "../base/malloc_allocator.cpp"
#include "../base/matrix4.cpp"
#include "../base/memory.cpp"
#include "../base/murmur_hash.cpp"
#include "../base/resource_path.cpp"
#include "../base/stream.cpp"
#include "../base/vector2.cpp"
#include "../base/vector2i.cpp"
#include "../base/vector2u.cpp"
#include "../base/vector4.cpp"
#include "../engine/engine.cpp"
#include "../engine/entity/components/component_header.cpp"
#include "../engine/entity/components/sprite_renderer_component.cpp"
#include "../engine/entity/components/transform_component.cpp"
#include "../engine/entity/entity.cpp"
#include "../engine/entity/entity_manager.cpp"
#include "../engine/font.cpp"
#include "../engine/keyboard.cpp"
#include "../engine/png.cpp"
#include "../engine/rect.cpp"
#include "../engine/renderer/renderer.cpp"
#include "../engine/renderer/render_interface.cpp"
#include "../engine/renderer/render_material.cpp"
#include "../engine/renderer/render_resource_table.cpp"
#include "../engine/renderer/render_uniform.cpp"
#include "../engine/renderer/render_world.cpp"
#include "../engine/resource_store.cpp"
#include "../engine/shader_utils.cpp"
#include "../engine/view.cpp"
#include "../engine/world.cpp"
#include "../game/game.cpp"
#include "../game/script_interfaces/script_console.cpp"
#include "../game/script_interfaces/script_engine.cpp"
#include "../game/script_interfaces/script_entity.cpp"
#include "../game/script_interfaces/script_interface_helpers.cpp"
#include "../game/script_interfaces/script_material.cpp"
#include "../game/script_interfaces/script_sprite_renderer_component.cpp"
#include "../game/script_interfaces/script_time.cpp"
#include "../game/script_interfaces/script_transform_component.cpp"
#include "../game/script_interfaces/script_world.cpp"
#include "../opengl_renderer/gl3w.c"
#include "../opengl_renderer/opengl_renderer.cpp"

namespace bowtie_windows
{
    bowtie::Engine* s_engine;
}

void window_resized_callback(const bowtie::Vector2u* resolution)
{
    bowtie::engine::resize(bowtie_windows::s_engine, resolution);
}

void key_down_callback(bowtie::Key key)
{
    bowtie::engine::key_pressed(bowtie_windows::s_engine, key);
}

void key_up_callback(bowtie::Key key)
{
    bowtie::engine::key_released(bowtie_windows::s_engine, key);
}

bowtie::PermanentMemory bowtie::MainThreadMemory;
bowtie::PermanentMemory bowtie::RenderThreadMemory;

void* alloc(bowtie::uint64 size)
{
    return VirtualAlloc(0, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

void dealloc(void* p)
{
    VirtualFree(p, 0, MEM_RELEASE);
}

DWORD WINAPI renderer_thread_proc(void* param)
{
    auto renderer = (bowtie::Renderer*)param;    
    bowtie::renderer::initialize_thread(renderer);

    while (renderer->active)
        bowtie::renderer::process_command_queue(renderer);

    return 0;
}

int main()
{
    // Alloc memory
    const auto permanent_memory_size = 33554432u; // 32 megabyte
    void* main_thread_memory_buffer = alloc(permanent_memory_size);
    bowtie::memory::init(&bowtie::MainThreadMemory, main_thread_memory_buffer, permanent_memory_size);
    void* render_thread_memory_buffer = alloc(permanent_memory_size);
    bowtie::memory::init(&bowtie::RenderThreadMemory, render_thread_memory_buffer, permanent_memory_size);
    const auto temp_memory_size = 134217728u; // 128 megabytes
    void* temp_memory_buffer = alloc(temp_memory_size);
    bowtie::temp_memory::init(temp_memory_buffer, temp_memory_size);
    auto callstack_capturer = bowtie::windows::callstack_capturer::create();
    auto allocator = new bowtie::MallocAllocator();
    bowtie::memory::init_allocator(allocator, "default allocator", &callstack_capturer);
    auto renderer_allocator = new bowtie::MallocAllocator();
    bowtie:: memory::init_allocator(renderer_allocator, "renederer allocator", &callstack_capturer);

    // Setup engine and renderer
    bowtie::Timer timer = {};
    timer.counter = bowtie::windows::timer::counter;
    timer.start = bowtie::windows::timer::start;
    auto opengl_renderer = bowtie::opengl_renderer::create();
    auto renderer_context = bowtie::windows::opengl_context::create();
    bowtie::Engine engine = {};
    bowtie::engine::init(&engine, allocator, &opengl_renderer, &renderer_context, renderer_allocator, &timer);
    bowtie_windows::s_engine = &engine;

    // Create window
    auto resolution = bowtie::vector2u::create(1280, 720);
    bowtie::windows::Window window = {};
    bowtie::windows::window::init(&window, GetModuleHandle(NULL), &resolution, &window_resized_callback, &key_down_callback, &key_up_callback);

    // Setup renderer thread
    bowtie::PlatformRendererContextData platform_renderer_context_data = {};
    bowtie::windows::opengl_context::init(&platform_renderer_context_data, window.hwnd);
    bowtie::renderer::setup(&engine.renderer, &platform_renderer_context_data, &resolution);
    auto render_thread = CreateThread(nullptr, 0, renderer_thread_proc, &engine.renderer, 0, nullptr);
    
    while(window.is_open)
    {
        bowtie::temp_memory::new_frame();
        bowtie::windows::window::dispatch_messages(&window);
        bowtie::engine::update_and_render(&engine);
    }

    bowtie::engine::deinit(&engine);
    bowtie::renderer::stop(&engine.renderer);
    WaitForSingleObject(render_thread, INFINITE);
    bowtie::renderer::deinit(&engine.renderer);

    // Dealloc memory
    bowtie::memory::deinit_allocator(renderer_allocator);
    bowtie::memory::deinit_allocator(allocator);
    dealloc(temp_memory_buffer);
    dealloc(main_thread_memory_buffer);
    dealloc(render_thread_memory_buffer);
}
