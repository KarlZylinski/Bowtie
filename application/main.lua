game = {}

function init()
    local engine = Engine.engine()
    game.world = World.create(engine)
    game.test_sprite = World.spawn_sprite(game.world, "beer.bmp")
end

function update(dt)
	local time = Time.time()
	local v = Sprite.position(game.test_sprite)

	if (v.x == 0) then
		v = Vector.new(200, 200)
	end

	Sprite.set_position(game.test_sprite, Vector.mul(1.00001, v))
    World.update(game.world)
    World.draw(game.world)
end

function deinit()
    local engine = Engine.engine()
    World.destroy(engine, game.world)
end