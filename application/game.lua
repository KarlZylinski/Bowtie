require "shared/class"
require "rocket"

Game = class(Game)

function Game:init()
    local engine = Engine.engine()
    Engine.load_resource("shader", "default.shader")
    Engine.set_default_resource("shader", "default.shader")
    self.world = World.create(engine)
    self.rocket = Rocket()
    self.rocket:spawn(self.world)
    self.view_pos = Vector2(0, 0)
    self.view_size = Vector2(1280, 720)
end

function Game:deinit()
    local engine = Engine.engine()
    World.destroy(engine, self.world)
end

function Game:update(dt)
    self.rocket:update(dt, self.view_size)
    World.update(self.world)
end

function Game:draw()
    World.draw(self.world, self.view_pos, self.view_size)
end

return Game
