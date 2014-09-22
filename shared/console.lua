local _truncate_array, _update_drawables

Console = class(Console)
console_height = 300
console_hidden_y_pos = -console_height
epsilon = 0.0001

function Console:init()
    local engine = Engine.engine()
    self.world = World.create(engine)
    self.font = Engine.load_resource("font", "shared/fonts/stolen.font")
    self.all_lines = {}
    self.visible_drawables = {}
    self.offset_from_bottom = 0
    self.current_y_pos = console_hidden_y_pos
    self.target_y_pos = console_hidden_y_pos
    local background_rectangle = Rectangle.spawn(self.world, Vector2(0, 0), Vector2(1280, console_height), Color(0.1, 0.1, 0.12, 1))
    Drawable.set_depth(background_rectangle, -100)
end

function Console:deinit()
    local engine = Engine.engine()
    World.destroy(engine, self.world)
end

function Console:write(message)
    max_lines_to_keep = 10000

    if self.offset_from_bottom ~= 0 then
        self.offset_from_bottom = self.offset_from_bottom + 1
    end

    table.insert(self.all_lines, message)

    if #self.all_lines > max_lines_to_keep then
        self.all_lines = _truncate_array(self.all_lines, max_lines_to_keep)
    end

    self.visible_drawables = _update_drawables(self.world, self.font, self.all_lines, self.visible_drawables, self.offset_from_bottom)
end

function Console:update(dt)
    local position_diff = self.target_y_pos - self.current_y_pos

    if math.abs(position_diff) > epsilon then
        self.current_y_pos = self.current_y_pos + position_diff * dt * 10
    else
        self.current_y_pos = self.target_y_pos
    end

    if not self:visible() then
        return
    end

    local offset_before = self.offset_from_bottom

    if Keyboard.pressed("PageUp") then
        local new_offset = self.offset_from_bottom + 7
        self.offset_from_bottom = new_offset < #self.all_lines and new_offset or #self.all_lines - 1
    end

    if Keyboard.pressed("PageDown") then
        local new_offset = self.offset_from_bottom - 7
        self.offset_from_bottom = new_offset > 0 and new_offset or 0
    end

    if offset_before ~= self.offset_from_bottom then
        self.visible_drawables = _update_drawables(self.world, self.font, self.all_lines, self.visible_drawables, self.offset_from_bottom)
    end

    World.update(self.world)
end

function Console:draw()
    if not self:visible() then
        return
    end

    World.draw(self.world, Vector2(0, self.current_y_pos), Vector2(1280, 720))
end

function Console:toggle()
    self.target_y_pos = self.target_y_pos ~= 0 and 0 or console_hidden_y_pos
end

function Console:visible()
    return math.abs(self.current_y_pos - console_hidden_y_pos) > epsilon
end

_truncate_array = function(array, num_to_keep)
    if #array <= num_to_keep then
        return array
    end

    local truncated = {}
    local first = #array - num_to_keep

    for i = first, #array do
        table.insert(truncated, array[i])
    end

    return truncated
end

_update_drawables = function(world, font, all_lines, visible_drawables, offset_from_bottom)
    line_height = 30
    max_lines_visible = console_height / line_height

    for _, drawable in pairs(visible_drawables) do
        Drawable.unspawn(world, drawable)
    end

    local drawables = {}

    if #all_lines == 0 then
        return drawables
    end

    local last = #all_lines - offset_from_bottom
    local first = last >= max_lines_visible and last - (max_lines_visible - 1) or 1
    local num_to_show = last - first + 1

    for i = first, last do
        local line = all_lines[i]
        local text = Text.spawn(world, font, line)
        Drawable.set_position(text, Vector2(5, 5 + (#drawables + (max_lines_visible - num_to_show)) * line_height))
        table.insert(drawables, text)
    end

    return drawables
end

return Console
