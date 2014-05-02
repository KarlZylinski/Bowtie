game = {}

function init()
    local engine = engine()
    game.world = create_world(engine)
end

function update(dt)
    update_world(game.world)
end

function deinit()
    local engine = engine()
    destroy_world(engine, game.world)
end