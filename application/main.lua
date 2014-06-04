game = {}

function init()
    local engine = Engine.engine()
    game.world = World.create(engine)
    game.test_sprite = World.spawn_sprite(game.world, "beer.bmp")
end

function update(dt)
	local time = Time.time();
	Sprite.set_position(game.test_sprite, math.cos(time) * 100, math.sin(time) * 100)
    World.update(game.world)
    World.draw(game.world)
end

function deinit()
    local engine = Engine.engine()
    World.destroy(engine, game.world)
end