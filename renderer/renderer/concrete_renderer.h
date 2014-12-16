#pragma once

#include <engine/render_resource_handle.h>
#include <engine/image.h>
#include <base/collection_types.h>
#include "render_resource.h"

namespace bowtie
{

struct GeometryResourceData;
struct RenderTarget;
struct RenderTexture;
struct RenderWorld;
struct ShaderResourceData;
struct TextureResourceData;
struct Rect;
struct Vector2u;

struct ConcreteRenderer
{
    // Initialization
    void (*initialize)();
    
    // Resource management
    RenderResource (*create_render_target)(const RenderTexture* texture);
    void (*destroy_render_target)(RenderResource render_target);
    uint32 (*get_uniform_location)(RenderResource shader, const char* name);
    RenderResource (*create_texture)(PixelFormat pf, const Vector2u* resolution, void* data);
    void (*destroy_texture)(RenderResource texture);
    RenderResource (*create_shader)(const char* vertex_source, const char* fragment_source);
    void (*destroy_shader)(RenderResource handle);
    RenderResource (*update_shader)(const RenderResource* shader, const char* vertex_source, const char* fragment_source);
        
    // State setters
    void (*resize)(const Vector2u* size, RenderTarget* render_targets);
    void (*set_render_target)(const Vector2u* resolution, RenderResource render_target);
    void (*unset_render_target)(const Vector2u* resolution);

    // Drawing
    void (*clear)();
    void (*draw)(const Rect* view, const RenderWorld* render_world, const Vector2u* resolution, real32 time, const RenderResource* resource_table);
    void (*combine_rendered_worlds)(RenderResource rendered_worlds_combining_shader, RenderWorld** rendered_worlds, uint32 num_rendered_worlds);
};

}
