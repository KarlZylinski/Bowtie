Vector4 = class(Vector4)
Color = Vector4

function Vector4:init(x, y, z, w)
    self.x = x or 0
    self.y = y or 0
    self.z = z or 1
    self.w = w or 1

    local mt = getmetatable(self)
    mt.__index = function(t, k)
        if k == 'r' then
            return t.x
        elseif k == 'g' then
            return t.y
        elseif k == 'b' then
            return t.z
        elseif k == 'a' then
            return t.w
        end

        return rawget(mt, k)
    end

    mt.__newindex = function(t, k, v)
        if k == 'r' then
            rawset(t, "x", v)
        elseif k == 'g' then
            rawset(t, "y", v)
        elseif k == 'b' then
            rawset(t, "z", v)
        elseif k == 'a' then
            rawset(t, "w", v)
        end

        rawset(t, k, v)
    end
end

function Vector4:__add(other)
    if type(other) == "number" then
        return Vector4(self.x + other, self.y + other, self.z + other, self.w + other)
    else
        return Vector4(self.x + other.x, self.y + other.y, self.z + other.z, self.w + other.w)
    end
end

function Vector4:__sub(other)
    if type(other) == "number" then
        return Vector4(self.x - other, self.y - other, self.z - other, self.w - other)
    else
        return Vector4(self.x - other.x, self.y - other.y, self.z - other.z, self.w - other.w)
    end
end

function Vector4:__mul(other)
    if type(other) == "number" then
        return Vector4(self.x * other, self.y * other, self.z * other, self.w * other)
    else
        return Vector4(self.x * other.x, self.y * other.y, self.z * other.z, self.w * other.w)
    end
end

function Vector4:__div(other)
    if type(other) == "number" then
        return Vector4(self.x / other, self.y / other, self.z / other, self.w / other)
    else
        return Vector4(self.x / other.x, self.y / other.y, self.z / other.z, self.w / other.w)
    end
end

function Vector4:__eq(other)
    return self.x == other.x and self.y == other.y and self.z == other.z and self.w == other.w
end

function Vector4:__tostring()
    return "[Vector4: " .. self.x .. ", " .. self.y .. ", " .. self.z ..", " .. self.w .."]"
end

function Vector4:distance(other)
    return (self - other):len()
end

function Vector4:dot(other)
    return self.x * other.x + self.y * other.y + self.z * other.z + self.w * other.w
end

function Vector4:clone()
    return Vector4(self.x, self.y, self.z, self.w)
end

function Vector4:unpack()
    return self.x, self.y, self.z, self.w
end

function Vector4:len()
    return math.sqrt(self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w)
end

function Vector4:len_sq()
    return self.x * self.x + self.y * self.y + self.z * self.z + self.w * self.w
end

function Vector4:normalize()
    local len = self:len()
    self.x = self.x / len
    self.y = self.y / len
    self.z = self.z / len
    self.w = self.w / len
    return self
end

function Vector4:normalized()
    return self / self:len()
end

function Vector4:round()
    self.x = math.round(self.x)
    self.y = math.round(self.y)
    self.w = math.round(self.z)
    self.w = math.round(self.w)
    return self
end

function Vector4:project_on(other)
    return (self * other) * other / other:len_sq()
end

return Vector4
