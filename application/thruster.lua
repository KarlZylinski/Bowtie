Thruster = class(Thruster)

function Thruster:init()
    self.material = Engine.load_resource("material", "fire.material")
    self._thrust = 0
end

function Thruster:spawn(world)
    self.sprite = Rectangle.spawn(world, Vector2(0,0), Vector2(30,50), Color(1, 1, 1, 1))
    local sprite_size = Tuple.second(Sprite.rect(self.sprite))
    Drawable.set_pivot(self.sprite, Vector2(sprite_size.x * 0.5, 0))    
    Drawable.set_material(self.sprite, self.material)
end

function Thruster:set_parent(parent)
    Drawable.set_parent(self.sprite, parent)
    local sprite_size = Tuple.second(Sprite.rect(self.sprite))
    local parent_size = Tuple.second(Sprite.rect(parent))
    Drawable.set_position(self.sprite, Vector2(0, parent_size.y * 0.5 - sprite_size.y * 0.5 - 3))
end

function Thruster:update(input, dt)
    self._thrust = input.y * 5000
    Material.set_uniform_value(self.material, "thrust", Vector4(self._thrust, 0, 0, 0))
end

function Thruster:thrust()
    return self._thrust
end
