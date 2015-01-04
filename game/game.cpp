#include "game.h"
#include <engine/engine.h>
#include <random>

namespace bowtie
{

namespace game
{

void init(Game* g, Allocator* allocator, Engine* engine, RenderInterface* render_interface)
{
    g->started = false;
}

void start(Game* g)
{
    g->started = true;
}

void deinit(Game* g)
{
    g->started = false;
 
}

void update(Game* g, real32 dt)
{
    if (!g->started)
        return;
}

void draw(Game* g)
{
    if (!g->started)
        return;
}

}

}
