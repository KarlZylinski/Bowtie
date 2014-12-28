require "fileutils"

def print_header(header)
    puts
    puts header
    puts "-" * header.length
end

puts "OK"
print_header "Creating unity build source include"

if !system("ruby write_source_include_header.rb bowtie_windows/source_include.h opengl_renderer")
    puts FAILED
    exit 1
end

puts "OK"
print_header "Setting up compiler and linker parameters"
libs_dir = ENV["BOWTIE_LIBS"]
source_dir = ENV["BOWTIE_SOURCE"]
output_dir = ENV["BOWTIE_OUTPUT"] || "bin"
FileUtils.mkdir_p(output_dir)
release_build = ARGV[0] == "release"
run = ARGV[0] == "run" or ARGV[1] == "run"

compiler_params = "/W4 /WX /TP " +
                  "/I #{libs_dir}/luajit/include " +
                  "/D WINDOWS /D GLEW_STATIC /DUNICODE /D_UNICODE /D _CRT_SECURE_NO_WARNINGS " +
                  "/wd4996 /wd4238 /wd4530 " +
                  "/I #{source_dir} " +
                  "/FI #{source_dir}/base/types.h /FI #{source_dir}/base/assert.h "

linker_params = "/subsystem:windows /entry:mainCRTStartup /incremental:no " +
                "/ignore:4098 " +
                "user32.lib lua51.lib opengl32.lib Gdi32.lib dbghelp.lib " +
                "/out:#{output_dir}/bowtie.exe"

if release_build 
    compiler_params = compiler_params + "/GL /O2 /MT /D NDEBUG"
    linker_params = "/libpath:#{libs_dir}/luajit/lib/debug " + linker_params
else
    compiler_params = compiler_params + "/Zi /MTd /D DEBUG"
    linker_params = "/libpath:#{libs_dir}/luajit/lib/release " + linker_params
end

puts "OK"
print_header "Compiling"
setup_build_environment = "\"" + ENV["VS110COMNTOOLS"] + "..\\..\\VC\\vcvarsall.bat\" amd64"
compiler_string = "#{setup_build_environment} && cl.exe #{compiler_params} bowtie_windows/bowtie_windows.cpp /link #{linker_params}"

if !system(compiler_string)
    puts "FAILED"
    exit 1
end

puts
puts "OK"

if run
    Dir.chdir(output_dir){
        system("bowtie.exe")
    }
end
