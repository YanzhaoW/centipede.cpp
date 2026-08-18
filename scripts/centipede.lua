local centipede = {}

---@diagnostic disable-next-line: undefined-global
centipede.config = get_default_config()

centipede.print = function(opts)
    for key, val in pairs(opts) do
        if type(val) == "table" then
            print(key .. ": ")
            centipede.print(val)
        else
            print(key .. ": " .. val)
        end
    end
end

local setup_table

local check_if_array = function(arr)
    if type(arr) ~= "table" then
        return false
    end

    for key, _ in pairs(arr) do
        if type(key) ~= "number" then
            return false
        end
    end
    return true
end


setup_table = function(from, to)
    if check_if_array(from) then
        for key, val in ipairs(from) do
            to[key] = val
        end
    end

    for key, _ in pairs(from) do
        if type(from[key]) == "table" then
            setup_table(from[key], to[key])
        else
            to[key] = from[key]
        end
    end
end

centipede.setup = function(opts)
    setup_table(opts, centipede.config)
end

return centipede
