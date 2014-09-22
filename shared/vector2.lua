Vector2 = class(Vector2)

function Vector2:init(x, y)
    self.x = x or 0
    self.y = y or 0
end

function Vector2:__add(other)
    if type(other) == "number" then
        return Vector2(self.x + other, self.y + other)
    else
        return Vector2(self.x + other.x, self.y + other.y)
    end
end

function Vector2:__sub(other)
    if type(other) == "number" then
        return Vector2(self.x - other, self.y - other)
    else
        return Vector2(self.x - other.x, self.y - other.y)
    end
end

function Vector2:__mul(other)
    if type(other) == "number" then
        return Vector2(self.x * other, self.y * other)
    else
        return Vector2(self.x * other.x, self.y * other.y)
    end
end

function Vector2:__div(other)
    if type(other) == "number" then
        return Vector2(self.x / other, self.y / other)
    else
        return Vector2(self.x / other.x, self.y / other.y)
    end
end

function Vector2:__eq(other)
    return self.x == other.x and self.y == other.y
end

function Vector2:__lt(other)
    return self.x < other.x or (self.x == other.x and self.y < other.y)
end

function Vector2:__le(other)
    return self.x <= other.x and self.y <= other.y
end

function Vector2:__tostring()
    return "[Vector2: " .. self.x .. ", " .. self.y .. "]"
end

function Vector2:distance(other)
    return (self - other):len()
end

function Vector2:dot(other)
    return self.x * other.x + self.y * other.y
end

function Vector2:clone()
    return Vector2(self.x, self.y)
end

function Vector2:unpack()
    return self.x, self.y
end

function Vector2:len()
    return math.sqrt(self.x * self.x + self.y * self.y)
end

function Vector2:len_sq()
    return self.x * self.x + self.y * self.y
end

function Vector2:normalize()
    local len = self:len()
    self.x = self.x / len
    self.y = self.y / len
    return self
end

function Vector2:normalized()
    return self / self:len()
end

function Vector2:rotate(phi)
    local c = math.cos(phi)
    local s = math.sin(phi)

    local x = self.x * c - self.y * s
    local y = self.x * s + self.y * c

    self.x = x
    self.y = y

    return self
end

function Vector2:rotate_deg(phi)
    return self:rotate(math.rad(phi))
end

function Vector2:rotated(phi)
    return self:clone():rotate(phi)
end

function Vector2:round()
    self.x = math.round(self.x)
    self.y = math.round(self.y)
    return self
end

function Vector2:perpendicular()
    return Vector2(-self.y, self.x)
end

function Vector2:project_on(other)
    return (self * other) * other / other:len_sq()
end

function Vector2:cross(other)
    return self.x * other.y - self.y * other.x
end

return Vector2
