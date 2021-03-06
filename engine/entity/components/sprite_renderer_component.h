#pragma once

#include "../../renderer/render_resource_handle.h"
#include "component_header.h"

namespace bowtie
{

struct Allocator;
struct RenderInterface;
struct Material;
struct Rect;
struct Vector4;
struct Matrix4;
struct Quad;
typedef Vector4 Color;

struct SpriteRendererComponentData
{
    Entity* entity;
    Color* color;
    Rect* rect;
    Material* material;
    RenderResourceHandle* render_handle;
    Quad* geometry;
    int32* depth;
};

struct SpriteRendererComponent
{
    ComponentHeader header;
    void* buffer;
    SpriteRendererComponentData data;
};

namespace sprite_renderer_component
{
    extern uint32 component_size;
    void init(SpriteRendererComponent* c);
    void create(Entity e, const Rect* rect, const Color* color);
    void destroy(SpriteRendererComponent* c, Entity e);
    void set_rect(SpriteRendererComponent* c, Entity e, const Rect* rect);
    const Rect* rect(SpriteRendererComponent* c, Entity e);
    void set_color(SpriteRendererComponent* c, Entity e, const Color* color);
    const Color* color(SpriteRendererComponent* c, Entity e);
    void set_render_handle(SpriteRendererComponent* c, Entity e, RenderResourceHandle render_handle);
    const Material* material(SpriteRendererComponent* c, Entity e);
    void set_material(Entity e, Material* material);
    RenderResourceHandle render_handle(SpriteRendererComponent* c, Entity e);
    void set_geometry(SpriteRendererComponent* c, Entity e, const Quad* geometry);
    const Quad* geometry(SpriteRendererComponent* c, Entity e);
    void set_depth(SpriteRendererComponent* c, Entity e, int32 depth);
    void* copy_dirty_data(SpriteRendererComponent* c);
    void* copy_new_data(SpriteRendererComponent* c);
    SpriteRendererComponentData create_data_from_buffer(void* buffer, uint32 num);
}

}
