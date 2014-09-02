require "shared/class"
require "shared/tuple"

local apply_gravity, apply_thrust, calculate_rotation, calculate_thrust, limit_velocity, limit_vertical_velocity, read_input

Rocket = class(Rocket)

function Rocket:init()
    Engine.load_resource("sprite", "sprites/rocket.sprite")
    self.fire_material = Engine.load_resource("material", "fire.material")
    self.velocity = Vector2(0, 0)
    self.thrust = 0
end

function Rocket:spawn(world)
    self.fire = Rectangle.spawn(world, Vector2(0,0), Vector2(30,50), Color(1, 1, 1, 0.5))
    self.sprite = Sprite.spawn(world, "sprites/rocket.sprite")
    Drawable.set_pivot(self.fire, Vector2(15, 25))
    Drawable.set_material(self.fire, self.fire_material)
    Drawable.set_position(self.sprite, Vector2(200, 0))
    Drawable.set_pivot(self.sprite, Tuple.second(Sprite.rect(self.sprite)) * 0.5);
end

function Rocket:update(dt, view_size)
    local input = read_input()
    local rotation = calculate_rotation(Drawable.rotation(self.sprite), input, dt)
    Drawable.set_rotation(self.sprite, rotation)
    self.velocity = apply_gravity(self.velocity, dt)
    self.thrust = calculate_thrust(self.thrust, input, dt)
    self.velocity = apply_thrust(self.velocity, self.thrust, rotation, dt)
    self.velocity = limit_velocity(self.velocity)
    local new_pos = Drawable.position(self.sprite) + self.velocity * dt
    local sprite_size = Tuple.second(Sprite.rect(self.sprite))

    if new_pos.y > view_size.y - sprite_size.y then
        new_pos.y = view_size.y - sprite_size.y
        self.velocity = Vector2(0, 0)
    end

    Material.set_uniform_value(self.fire_material, "thrust", Vector4(self.velocity.y, 0, 0, 0))
    Drawable.set_position(self.sprite, new_pos)
    Drawable.set_position(self.fire, new_pos + Vector2(0, sprite_size.y * 0.5 - 3))
end

apply_gravity = function(current_velocity, dt)
    return current_velocity + Vector2(0, 9.82 * 300 * dt);
end

apply_thrust = function(current_velocity, current_thrust, rotation, dt)
    local vertical_velocity_change = current_thrust * dt
    return current_velocity + Vector2(-vertical_velocity_change * math.sin(rotation),
        vertical_velocity_change * math.cos(rotation))
end

calculate_rotation = function(current_rotation, input, dt)
    return current_rotation + input.x * 5 * dt
end

calculate_thrust = function(current_thrust, input, dt)
    return input.y * 5000
end

limit_velocity = function(current_velocity)
    return Vector2(current_velocity.x, limit_vertical_velocity(current_velocity.y))
end

limit_vertical_velocity = function(current_vertical_velocity)
    if current_vertical_velocity < -1000 then
        return -1000
    elseif current_vertical_velocity > 1000 then
        return 1000
    end

    return current_vertical_velocity
end

read_input = function()
    local input = Vector2(0, 0)

    if Keyboard.held("Up") then
        input.y = -1
    end

    if Keyboard.held("Down") then
        input.y = 1
    end

    if Keyboard.held("Left") then
        input.x = input.x - 1
    end

    if Keyboard.held("Right") then
        input.x = input.x + 1
    end

    return input
end

return Rocket
