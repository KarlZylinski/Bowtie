require "game"

local game

function init()
    game = Game()
end

function update(dt)
    if (Keyboard.pressed("Tilde")) then
        console:toggle()
    end

    game:update(dt)
end

function draw()
    game:draw()
end

function deinit()
    game = nil
end
