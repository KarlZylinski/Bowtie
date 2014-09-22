require "shared/math"

Thruster = class(Thruster)

function Thruster:init()
    self._material = Engine.load_resource("material", "fire.material")
    self._thrust = 0
    self._flare = 0
end

function Thruster:spawn(world)
    self._sprite = Rectangle.spawn(world, Vector2(0,0), Vector2(30,80), Color(1, 1, 1, 1), 20)
    local sprite_size = Tuple.second(Sprite.rect(self._sprite))
    Drawable.set_pivot(self._sprite, Vector2(sprite_size.x * 0.5, 0))
    Drawable.set_material(self._sprite, self._material)
end

function Thruster:set_parent(parent)
    Drawable.set_parent(self._sprite, parent)
    local sprite_size = Tuple.second(Sprite.rect(self._sprite))
    local parent_size = Tuple.second(Sprite.rect(parent))
    Drawable.set_position(self._sprite, Vector2(0, parent_size.y * 0.5 - sprite_size.y * 0.5 - 3))
end

function Thruster:update(input, dt)
    max_thrust = 5000
    self._thrust = input.y * max_thrust
    self._flare = self._flare - dt * 4
    self._flare = self._flare - input.y * dt * 20
    self._flare = clamp(self._flare, 0, 1)
    Material.set_uniform_value(self._material, "thrust", Vector4(self._flare, 0, 0, 0))
end

function Thruster:thrust()
    return self._thrust
end
