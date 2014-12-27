require "lfs"

forceincludes = { lfs.currentdir() .. "/base/base/types.h", lfs.currentdir() .. "/base/base/assert.h" }

local function platform_windows(configuration)
    local compiler_string = "cl.exe /c /wd4996 /wd4238 /TP"

    for _, include in pairs(forceincludes) do
        compiler_string = string.format("%s /FI %s", compiler_string, include)
    end

    if configuration == "debug" then
        compiler_string = compiler_string .. " /MTd"
    else
        compiler_string = compiler_string .. " /MT"
    end

    local function get_compile_command(file)
        return string.format("%s %s /out:%s", compiler_string, file, file .. ".o")
    end

    return {
        get_compile_command = get_compile_command,
        entry_project = "application_windows",
        extra_modules = {"opengl_renderer"}
    }
end

function build(platform)
    local modules = { "bowtie_windows", "base", "engine", "game", "os" }

    for _, m in pairs(platform.extra_modules) do
        table.insert(modules, m)
    end 
    
    local function is_source_file(filename)
        local len = string.len(filename)
        return (filename:sub(len-3, len-3) == "." and filename:sub(len-2, len-2) == "c" and filename:sub(len-1, len-1) == "p" and filename:sub(len, len) == "p")
            or (filename:sub(len-1, len-1) == "." and filename:sub(len, len) == "c")
    end

    local function enumerate_files(folder)
        local files = {}
        
        for file in lfs.dir(folder) do
            local path = string.format("%s/%s", folder, file)
            local is_directory = lfs.attributes(path, "mode") == "directory"
            if is_directory and file ~= "." and file ~= ".." then
                local files_in_subdirectory = enumerate_files(path)

                if files_in_subdirectory ~= nil then
                    for _, f in pairs(files_in_subdirectory) do
                        table.insert(files, f)
                    end
                end
            elseif is_source_file(path) then
                table.insert(files, path)
            end
        end

        return files
    end

    for _, m in pairs(modules) do
        local get_module_extra_info = loadfile(string.format("%s/build.lua", m))
        local build_info = get_module_extra_info and get_module_extra_info()
        local source_files = enumerate_files(m)

        for _, s in pairs(source_files) do
            print(s)
        end
    end
end

build(platform_windows())
