#pragma once

namespace bowtie
{

struct Allocator;
struct Engine;
struct RenderInterface;
struct World;

struct Game
{
    bool started;
    Allocator* allocator;
    Engine* engine;
    RenderInterface* render_interface;
    World* world;
    Vector2 view_pos;
};

namespace game
{
    void init(Game* g, Allocator* allocator, Engine* engine, RenderInterface* render_interface);
    void start(Game* g);
    void deinit(Game* g);
    void update(Game* g, real32 dt);
    void draw(Game* g);
    void deinit(Game* g);
}

} // namespace bowtie
