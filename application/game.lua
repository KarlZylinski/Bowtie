require "rocket"

Game = class(Game)

function Game:init()
    local engine = Engine.engine()
    Engine.load_resource("material", "default.material")
    Engine.set_default_resource("material", "default.material")
    self._world = World.create(engine)
    self._rocket = Rocket()
    self._rocket:spawn(self._world)
    self._view_pos = Vector2(0, 0)
    self._view_size = Vector2(1280, 720)
end

function Game:deinit()
    local engine = Engine.engine()
    World.destroy(engine, self._world)
end

function Game:update(dt)
    self._rocket:update(dt, self._view_size)
    self._view_pos = self._rocket:position() * -1 + self._view_size * 0.5
    World.update(self._world)
end

function Game:draw()
    World.draw(self._world, self._view_pos, self._view_size)
end

return Game
