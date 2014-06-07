game = {}

function init()
    local engine = Engine.engine()

    Engine.load_resource("shader", "default.shader")
	Engine.set_default_resource("shader", "default.shader")

	Engine.load_resource("sprite", "textures/beer.png")
	Engine.load_resource("sprite", "textures/hill.png")
	Engine.load_resource("sprite", "fonts/stolen.png")

    game.world = World.create(engine)
    game.hill = World.spawn_sprite(game.world, "textures/hill.png")
    Sprite.set_position(game.hill, Vector2(50, 50))
    game.beer = World.spawn_sprite(game.world, "textures/beer.png")
    game.text = World.spawn_sprite(game.world, "fonts/stolen.png")
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

	Sprite.set_position(game.beer, Sprite.position(game.beer) + direction * dt * 400)
end

function update(dt)
	update_beer(dt)
    World.update(game.world)
    World.draw(game.world)
end

function deinit()
    local engine = Engine.engine()
    World.destroy(engine, game.world)
end
