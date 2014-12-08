#pragma once

struct lua_State;

namespace bowtie
{

class Allocator;
struct Engine;
struct RenderInterface;

struct Game
{
	bool started;
	lua_State* _lua;
};

namespace game
{
	void init(Game& g, Allocator& allocator, Engine& engine, RenderInterface& render_interface);
	void start(Game& g);
	void deinit(Game& g);
	void update(Game& g, float dt);
	void draw(Game& g);
	void deinit(Game& g);
}

} // namespace bowtie
