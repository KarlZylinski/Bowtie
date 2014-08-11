require "shared/class"

local _update_drawables

Console = class(Console)

function Console:init()
    local engine = Engine.engine()
    self.world = World.create(engine)
    self.font = Engine.load_resource("font", "fonts/stolen.font")
    self.all_lines = {}
    self.visible_drawables = {}
    Rectangle.spawn(self.world, Vector2(0,0), Vector2(1280, 360), Color(0.1, 0.1, 0.12, 1))
end

function Console:deinit()
    local engine = Engine.engine()
    World.destroy(engine, self.world)
end

function Console:write(message)
    table.insert(self.all_lines, message)
    self.visible_drawables = _update_drawables(self.world, self.font, self.all_lines, self.visible_drawables)
end

function Console:update(dt)
    World.update(self.world)

--[[
    if Keyboard.pressed("Z") then
        local k, row = next(self.rows, 2)
        table.remove(self.rows, k)
        Drawable.unspawn(self.world, row)
    end]]
end

function Console:draw()
    World.draw(self.world, Vector2(0, 0), Vector2(1280, 720))
end

_update_drawables = function(world, font, all_lines, visible_drawables)
    max_lines_visible = 12

    for _, drawable in pairs(visible_drawables) do
        Drawable.unspawn(world, drawable)
    end

    local drawables = {}

    if #all_lines == 0 then
        return drawables
    end

    local last = #all_lines
    local first = last >= max_lines_visible and last - (max_lines_visible - 1) or 1

    for i = first, last do
        local line = all_lines[i]
        local text = Text.spawn(world, font, line)
        Drawable.set_position(text, Vector2(0, #drawables * 30))
        table.insert(drawables, text)
    end

    return drawables
end

return Console
