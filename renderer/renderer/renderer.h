#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>

#include <engine/renderer_command.h>
#include <engine/render_interface.h>
#include <engine/render_resource_types.h>
#include <base/collection_types.h>
#include <base/concurrent_ring_buffer.h>
#include "render_resource_table.h"
#include "render_resource.h"
#include <os/renderer_context.h>
#include "render_world.h"
#include "concrete_renderer.h"
#include "constants.h"

namespace bowtie
{

struct ConcreteRenderer;
struct RenderTarget;

struct RendererResourceObject
{
    RendererResourceObject() {}
    RendererResourceObject(RenderResourceData::Type type, RenderResourceHandle handle)
        : type(type), handle(handle) {}

    RenderResourceData::Type type;
    RenderResourceHandle handle;
};

struct CreatedResources {
    uint32 num;
    RenderResourceHandle* handles;
    RenderResource* resources;
};

struct UpdatedResources {
    uint32 num;
    RenderResourceHandle* handles;
    RenderResource* old_resources;
    RenderResource* new_resources;
};

struct Renderer
{
    bool active;
    Allocator* allocator;
    ConcreteRenderer _concrete_renderer;
    RendererContext _context;
    PlatformRendererContextData* _context_data;
    RenderInterface render_interface;
    Vector2u resolution;
    RenderResource resource_table[render_resource_handle::num];
    RenderTarget _render_targets[renderer::max_render_targets];
    RenderWorld* _rendered_worlds[renderer::max_rendered_worlds];
    uint32 num_rendered_worlds;
    RendererResourceObject _resource_objects[render_resource_handle::num]; // Same amount of maximum resource objects as handles.
    RenderResource _rendered_worlds_combining_shader;
    std::thread _thread;
    ConcurrentRingBuffer _unprocessed_commands;
    std::mutex _unprocessed_commands_exist_mutex;
    std::condition_variable _wait_for_unprocessed_commands_to_exist;
    bool _unprocessed_commands_exist;
};

namespace renderer
{
    void init(Renderer* r, const ConcreteRenderer* concrete_renderer_obj, Allocator* renderer_allocator, const RendererContext* context);
    void deinit(Renderer* r);
    void run(Renderer* r, PlatformRendererContextData* context, const Vector2u* resolution);
    void stop(Renderer* r);
};

}
