#include "game.h"
#include <engine/engine.h>
#include <engine/resource_store.h>

namespace bowtie
{

namespace game
{

const Vector2 view_size = vector2::create(1280, 720);

void init(Game* g, Allocator* allocator, Engine* engine, RenderInterface* render_interface)
{
    g->started = false;
    g->allocator = allocator;
    g->engine = engine;
    g->render_interface = render_interface;
    g->view_pos.x = g->view_pos.y = 0;
}

void start(Game* g)
{
    g->started = true;
    auto default_material = resource_store::load(&g->engine->resource_store, ResourceType::Material, "default.material");

    if (default_material.is_some)
        resource_store::set_default(&g->engine->resource_store, ResourceType::Material, default_material.value);

    g->world = engine::create_world(g->engine);
    auto background_material = resource_store::load(&g->engine->resource_store, ResourceType::Material, "background.material");
    auto background = entity_manager::create(&g->engine->entity_manager, g->world);
    transform_component::create(background);
    transform_component::set_position(background, &vector2::create(0, 0));
    sprite_renderer_component::create(background, &rect::create(&vector2::create(0, 0), &view_size), &vector4::create(1, 1, 1, 1));
    sprite_renderer_component::set_material(background, (Material*)option::get(background_material));
}

void deinit(Game* g)
{
    engine::destroy_world(g->engine, g->world);
    g->started = false;
}

void update(Game* g, real32 dt)
{
    if (!g->started)
        return;

    world::update(g->world);
}

void draw(Game* g)
{
    if (!g->started)
        return;

    auto view = rect::create(&vector2::create(0, 0), &view_size);
    world::draw(g->world, &view, g->engine->timer->counter()); 
}

}

}
