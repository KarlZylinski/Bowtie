#pragma once

struct lua_State;

namespace bowtie
{

class Allocator;
class Engine;

class Game
{
public:
	Game(Allocator& allocator, Engine& engine);
	~Game();

	void init();
	void update(float dt);
	void deinit();

	bool initialized() const;

private:
	Allocator& _allocator;
	Engine& _engine;
	bool _initialized;
	lua_State* _lua_state;

	Game(const Game&);
	Game& operator=(const Game&);
};

} // namespace bowtie
