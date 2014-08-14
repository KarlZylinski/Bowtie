require "shared/class"

local apply_gravity, apply_thrust, calculate_rotation, calculate_thrust, limit_velocity, limit_vertical_velocity, read_input

Rocket = class(Rocket)

function Rocket:init()
    Engine.load_resource("sprite", "textures/rocket.png")
    self.velocity = Vector2(0, 0)
    self.thrust = 0
end

function Rocket:spawn(world)
    self.sprite = Sprite.spawn(world, "textures/rocket.png")
    Drawable.set_position(self.sprite, Vector2(200, 0))
end

function Rocket:update(dt, view_size)
    local input = read_input()
    Drawable.set_rotation(self.sprite, calculate_rotation(Drawable.rotation(self.sprite), input, dt))
    self.velocity = apply_gravity(self.velocity, dt)
    self.thrust = calculate_thrust(self.thrust, input, dt)
    self.velocity = apply_thrust(self.velocity, self.thrust, dt)
    self.velocity = limit_velocity(self.velocity)
    local new_pos = Drawable.position(self.sprite) + self.velocity * dt
    local _, sprite_size = Sprite.rect(self.sprite)

    if new_pos.y > view_size.y - sprite_size.y then
        new_pos.y = view_size.y - sprite_size.y
        self.velocity.y = 0
    end

    Drawable.set_position(self.sprite, new_pos)
end

apply_gravity = function(current_velocity, dt)
    return current_velocity + Vector2(0, 9.82 * 300 * dt);
end

apply_thrust = function(current_velocity, current_thrust, dt)
    local vertical_velocity_change = current_thrust * dt
    return current_velocity + Vector2(0, vertical_velocity_change)
end

calculate_rotation = function(current_rotation, input, dt)
    return current_rotation + input.x * dt
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
