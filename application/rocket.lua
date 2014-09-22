require "shared/tuple"
require "thruster"

local apply_gravity, apply_thrust, calculate_rotation, calculate_thrust, limit_velocity, limit_vertical_velocity, read_input

Rocket = class(Rocket)

function Rocket:init()
    Engine.load_resource("sprite", "sprites/rocket.sprite")
    self._thruster = Thruster()
    self._velocity = Vector2(0, 0)
end

function Rocket:spawn(world)
    self._thruster:spawn(world)
    self._sprite = Sprite.spawn(world, "sprites/rocket.sprite", 50)
    self._thruster:set_parent(self._sprite)
    local sprite_size = Tuple.second(Sprite.rect(self._sprite))
    Drawable.set_position(self._sprite, Vector2(200, 0))
    Drawable.set_pivot(self._sprite, sprite_size * 0.5);
end

function Rocket:update(dt, view_size)
    local input = read_input()
    local rotation = calculate_rotation(Drawable.rotation(self._sprite), input, dt)
    Drawable.set_rotation(self._sprite, rotation)
    self._velocity = apply_gravity(self._velocity, dt)
    self._thruster:update(input, dt)
    self._velocity = apply_thrust(self._velocity, self._thruster:thrust(), rotation, dt)
    self._velocity = limit_velocity(self._velocity)
    local new_pos = Drawable.position(self._sprite) + self._velocity * dt
    local sprite_size = Tuple.second(Sprite.rect(self._sprite))

    if new_pos.y > view_size.y - sprite_size.y then
        new_pos.y = view_size.y - sprite_size.y
        self._velocity = Vector2(0, 0)
    end

    console:write(new_pos.y)
    Drawable.set_position(self._sprite, new_pos)
end

function Rocket:position()
    return Drawable.position(self._sprite)
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
