#pragma once
#include <game/game.h>
#include "keyboard.h"
#include "resource_store.h"
#include "entity/entity_manager.h"
#include "renderer/renderer.h"

namespace bowtie
{

struct Allocator;
struct RendererContext;
struct RenderInterface;
struct World;
struct Timer;
struct Vector2u;

struct Engine
{
    Allocator* allocator;
    EntityManager entity_manager;
    ResourceStore resource_store;
    Renderer renderer;
    Keyboard keyboard;
    Game _game;
    real32 _time_elapsed_previous_frame;
    real32 _time_since_start;
    Timer* timer;
};

namespace engine
{
    void init(Engine* e, Allocator* allocator, ConcreteRenderer* concrete_renderer,
              RendererContext* renderer_context, Allocator* renderer_allocator, Timer* timer);
    void deinit(Engine* e);
    World* create_world(Engine* e);
    void destroy_world(Engine* e, World* world);
    const Keyboard* keyboard(const Engine* e);
    void key_pressed(Engine* e, Key key);
    void key_released(Engine* e, Key key);
    void resize(Engine* e, const Vector2u* resolution);
    void update_and_render(Engine* e);

}

}