require "shared/class"
require "rocket"

Game = class(Game)

function Game:init()
    local engine = Engine.engine()
    self.default_material = Engine.load_resource("material", "default.material")
    Engine.set_default_resource("material", "default.material")
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

t = 0

function Game:update(dt)
    t = t + dt
    Material.set_uniform_value(self.default_material, "test", Vector4(math.cos(t), 0, 0, 0))
    self.rocket:update(dt, self.view_size)
    World.update(self.world)
end

function Game:draw()
    World.draw(self.world, self.view_pos, self.view_size)
end

return Game
