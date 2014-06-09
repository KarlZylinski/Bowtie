game = {}

function init()
    local engine = Engine.engine()

    Engine.load_resource("shader", "default.shader")
	Engine.set_default_resource("shader", "default.shader")

	Engine.load_resource("sprite", "textures/beer.png")
	Engine.load_resource("sprite", "textures/hill.png")
	game.font = Engine.load_resource("font", "fonts/stolen.png")

    game.world = World.create(engine)
    game.hill = World.spawn_sprite(game.world, "textures/hill.png")
    Drawable.set_position(game.hill, Vector2(50, 50))
    game.beer = World.spawn_sprite(game.world, "textures/beer.png")
    game.text = World.spawn_text(game.world, game.font, "Sexy pung")
end

function update_beer(dt)
	local direction = Vector2()

	if Keyboard.held("Up") then
		direction.y = -1
	end

	if Keyboard.held("Down") then
		direction.y = 1
	end

	if Keyboard.held("Left") then
		direction.x = -1
	end

	if Keyboard.held("Right") then
		direction.x = 1
	end

	Drawable.set_position(game.beer, Drawable.position(game.beer) + direction * dt * 400)
end

local t = 0

function update(dt)
	local color = Drawable.color(game.text)

	local pos = Drawable.position(game.beer)

	Drawable.set_color(game.beer, Vector4((math.sin(pos.x)+1)/2,(math.sin(pos.y)+1)/2,0,1))
	Drawable.set_color(game.text, Vector4((math.cos(pos.x)+1)/2,(math.cos(pos.y)+1)/2,0,1))
	Drawable.set_color(game.hill, Vector4((math.cos(pos.y)+1)/2,(math.sin(pos.x)+1)/2,0,1))
	Drawable.set_position(game.hill, Vector2(((math.cos(pos.y)+1)/2) * 640 - 200, ((math.sin(pos.x)+1)/2) * 360 - 200))

	update_beer(dt)
    World.update(game.world)
    World.draw(game.world)
end

function deinit()
    local engine = Engine.engine()
    World.destroy(engine, game.world)
end
