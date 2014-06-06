game = {}

function init()
    local engine = Engine.engine()
    game.world = World.create(engine)
    game.test_sprite = World.spawn_sprite(game.world, "beer.bmp")
end

function update_test_sprite(dt)
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

	Sprite.set_position(game.test_sprite, Sprite.position(game.test_sprite) + direction * dt * 400)
end

function update(dt)
	local time = Time.time()
	local v = Sprite.position(game.test_sprite)

	update_test_sprite(dt)
    World.update(game.world)
    World.draw(game.world)
end

function deinit()
    local engine = Engine.engine()
    World.destroy(engine, game.world)
end