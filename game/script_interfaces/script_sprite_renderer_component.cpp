#include "script_entity.h"
#include <lua.hpp>
#include <engine/entity/entity_manager.h>
#include <engine/world.h>
#include <base/murmur_hash.h>
#include <base/vector4.h>
#include "script_interface_helpers.h"
#include "script_console.h"
#include <engine/rect.h>
#include <engine/world.h>

namespace bowtie
{

namespace sprite_renderer_component_script_interface
{

namespace
{
    Allocator* s_allocator = nullptr;
}

int32 create(lua_State* lua)
{
    auto e = script_interface::to_entity(lua, 1);
    
    {
        auto transform = &e.world->transform_components;
        if (!component::has_entity(&transform->header, e))
            transform_component::create(transform, e);
    }

    Vector2 position;

    if (script_interface::is_vector2(lua, 2))
        position = script_interface::to_vector2(lua, 2);

    Vector2 size;

    if (script_interface::is_vector2(lua, 3))
        size = script_interface::to_vector2(lua, 3);

    Color color = {1, 1, 1, 1};

    if (script_interface::is_color(lua, 4))
        color = script_interface::to_color(lua, 4);

    Rect rect;
    rect::init(&rect, &position, &size);
    sprite_renderer_component::create(&e.world->sprite_renderer_components, e, &rect, &color);
    return 0;
}

int32 destroy(lua_State* lua)
{
    auto e = script_interface::to_entity(lua, 1);
    sprite_renderer_component::destroy(&e.world->sprite_renderer_components, e);
    return 0;
}

int32 rect(lua_State* lua)
{
    auto e = script_interface::to_entity(lua, 1);
    auto rect = sprite_renderer_component::rect(&e.world->sprite_renderer_components, e);
    script_interface::push_vector2(lua, &rect->position);
    script_interface::push_vector2(lua, &rect->size);
    return 2;
}

int32 set_rect(lua_State* lua)
{
    auto e = script_interface::to_entity(lua, 1);
    auto position = script_interface::to_vector2(lua, 2);
    auto size = script_interface::to_vector2(lua, 3);
    Rect rect;
    rect::init(&rect, &position, &size);
    sprite_renderer_component::set_rect(&e.world->sprite_renderer_components, e, &rect);
    return 0;
}

int32 color(lua_State* lua)
{
    auto e = script_interface::to_entity(lua, 1);
    auto color = sprite_renderer_component::color(&e.world->sprite_renderer_components, e);
    script_interface::push_color(lua, color);
    return 1;
}

int32 set_color(lua_State* lua)
{
    auto e = script_interface::to_entity(lua, 1);
    auto color = script_interface::to_color(lua, 2);
    sprite_renderer_component::set_color(&e.world->sprite_renderer_components, e, &color);
    return 0;
}

int32 set_material(lua_State* lua)
{
    auto e = script_interface::to_entity(lua, 1);
    auto material = (Material*)lua_touserdata(lua, 2);
    sprite_renderer_component::set_material(&e.world->sprite_renderer_components, e, material);
    return 0;
}

int32 set_depth(lua_State* lua)
{
    auto e = script_interface::to_entity(lua, 1);
    sprite_renderer_component::set_depth(&e.world->sprite_renderer_components, e, (int)lua_tonumber(lua, 2));
    return 0;
}

void load(lua_State* lua, Allocator* allocator)
{
    s_allocator = allocator;

    const interface_function functions[] = {
        { "create", create },
        { "destroy", destroy },
        { "rect", rect },
        { "set_rect", set_rect },
        { "color", color },
        { "set_color", set_color },
        { "set_material", set_material },
        { "set_depth", set_depth }
    };

    script_interface::register_interface(lua, "SpriteRenderer", functions, 8);
}

} // namespace sprite_renderer_component_script_interface
} // namespace bowtie