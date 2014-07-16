function class(klass, super)
    if not klass then
        klass = {} 
        local meta = {}

        meta.__call = function(self, ...)
            local object = {}
            setmetatable(object, klass)
            if object.init then object:init(...) end

            local proxy = newproxy(true)
            local proxy_meta = getmetatable(proxy)
            
            proxy_meta.__gc = function()
                if object.deinit then object:deinit() end
            end

            rawset(klass, "__gc", proxy);

            return object
        end

        setmetatable(klass, meta)
    end
    
    if super then
        for k,v in pairs(super) do
            klass[k] = v
        end
    end
    klass.__index = klass
    
    return klass
end
