Tuple = {}

function Tuple.first(...)
    return select(1, ...)
end

function Tuple.second(...)
    return select(2, ...)
end

function Tuple.third(...)
    return select(3, ...)
end

function Tuple.fourth(...)
    return select(4, ...)
end