require "fileutils"

if !system("ruby write_source_include_header.rb bowtie_windows/source_include.h opengl_renderer")
    puts "Failed creating unity build source include"
    exit 1
end

libs_dir = ENV["BOWTIE_LIBS"]
source_dir = ENV["BOWTIE_SOURCE"]
output_dir = ENV["BOWTIE_OUTPUT"] || "bin"
FileUtils.mkdir_p(output_dir)
release_build = ARGV[0] == "release"
run = ARGV[0] == "run" or ARGV[1] == "run"

compiler_params = "/W4 /WX /TP " +
                  "/D WINDOWS /D GLEW_STATIC /DUNICODE /D_UNICODE /D _CRT_SECURE_NO_WARNINGS " +
                  "/wd4996 /wd4238 /wd4530 /wd4100 " +
                  "/I #{source_dir} " +
                  "/FI #{source_dir}/base/types.h /FI #{source_dir}/base/assert.h "

linker_params = "/subsystem:windows /entry:mainCRTStartup /incremental:no " +
                "/ignore:4098 " +
                "user32.lib opengl32.lib Gdi32.lib dbghelp.lib " +
                "/out:#{output_dir}/bowtie.exe"

if release_build 
    compiler_params = compiler_params + "/GL /O2 /MT /D NDEBUG"
else
    compiler_params = compiler_params + "/Zi /MTd /D DEBUG"
end

setup_build_environment = "\"" + ENV["VS110COMNTOOLS"] + "..\\..\\VC\\vcvarsall.bat\" amd64"
compiler_string = "#{setup_build_environment} && cl.exe #{compiler_params} bowtie_windows/bowtie_windows.cpp /link #{linker_params}"

if !system(compiler_string)
    exit 1
end

if run
    Dir.chdir(output_dir){
        system("bowtie.exe")
    }
end
