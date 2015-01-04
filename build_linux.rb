require "fileutils"

def print_header(header)
    puts
    puts header
    puts "-" * header.length
end

puts "OK"
print_header "Creating unity build source include"

if !system("ruby write_source_include_header.rb bowtie_linux/source_include.h opengl_renderer")
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

compiler_params =  "-std=c++11 -x c++ -Wall -Werror " +
                   "-Wno-unknown-pragmas -Wno-address-of-temporary -Wno-missing-braces " +
                   "-D LINUX -D GLEW_STATIC " +
                   "-I #{source_dir} " +
                   "-include #{source_dir}/base/types.h -include #{source_dir}/base/assert.h "

linker_params = "-L/usr/X11R6/lib -ldl -lX11 -lXft -lGL -o #{output_dir}/bowtie "
#                "user32.lib lua51.lib opengl32.lib Gdi32.lib dbghelp.lib " +

if release_build 
    #compiler_params = compiler_params + "/GL /O2 /MT /D NDEBUG"
    #linker_params = "/libpath:#{libs_dir}/luajit/lib/debug " + linker_params
else
    #compiler_params = compiler_params + "/Zi /MTd /D DEBUG"
    #linker_params = "/libpath:#{libs_dir}/luajit/lib/release " + linker_params
end

puts "OK"
print_header "Compiling"
#setup_build_environment = "\"" + ENV["VS110COMNTOOLS"] + "..\\..\\VC\\vcvarsall.bat\" amd64"
#compiler_string = "#{setup_build_environment} && cl.exe #{compiler_params} bowtie_windows/bowtie_windows.cpp /link #{linker_params}"
compiler_string = "clang++ #{compiler_params} bowtie_linux/bowtie_linux.cpp #{linker_params}"

puts compiler_string

if !system(compiler_string)
    puts "FAILED"
    exit 1
end

puts
puts "OK"

if run
    Dir.chdir(output_dir){
        system("./bowtie")
    }
end
