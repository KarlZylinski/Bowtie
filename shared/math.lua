function min(x, y)
    if x < y then
        return x
    else
        return y
    end
end

function max(x, y)
    if x > y then
        return x
    else
        return y
    end
end

function clamp(val, min_val, max_val)
    return max(min_val, min(val, max_val))
end