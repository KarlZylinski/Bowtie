require "shared/class"

Console = class(Console)

function Console:init()
	local engine = Engine.engine()
	self.world = World.create(engine)
	self.font = Engine.load_resource("font", "fonts/stolen.png")
	self.rows = {}
end

function Console:deinit()
	local engine = Engine.engine()
	World.destroy(engine, self.world)
end

function Console:write(message)
	local text = World.spawn_text(self.world, self.font, message)
	Drawable.set_position(text, Vector2(0, #self.rows * 30))
	table.insert(self.rows, text)
end

function Console:update(dt)
	World.update(self.world)
end

function Console:draw()
	World.draw(self.world, Vector2(0, 0), Vector2(1280, 720))
end

return Console
