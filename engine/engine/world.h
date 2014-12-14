#pragma once

#include <base/collection_types.h>
#include "render_resource_handle.h"
#include "entity/components/sprite_renderer_component.h"
#include "entity/components/transform_component.h"

namespace bowtie
{

struct Rect;
struct Material;
struct RenderInterface;
struct ResourceStore;

struct World
{
    Allocator* allocator;
    RenderResourceHandle render_handle;
    RenderInterface* render_interface;
    RenderResourceHandle default_material;
    TransformComponent transform_components;
    SpriteRendererComponent sprite_renderer_components;
};

namespace world
{
    void init(World* w, Allocator* allocator, RenderInterface* render_interface, ResourceStore* resource_store);
    void deinit(World* w);
    void update(World* w);
    void draw(World* w, const Rect* view, real32 time);
}

};
