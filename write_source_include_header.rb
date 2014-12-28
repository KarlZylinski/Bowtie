if ARGV.empty?
    puts "Usage: write_build_include_header.rb bowtie_windows/source_include.h opengl_renderer"
    exit
end

output = ARGV[0]

default_folders = [
    "base",
    "engine",
    "game",
    "os"
]

extra_folders = ARGV.slice(1, ARGV.count) || []
all_folders = default_folders + extra_folders
all_source_files = (Dir.glob("**/*.c") + Dir.glob("**/*.cpp")).select{|file|
    all_folders.any?{|folder| file.start_with? folder}
}
all_source_files_relative = all_source_files.map{|f| "#include \"../" + f + "\""}.join("\n")
output_string = "#pragma once\n" + all_source_files_relative
File.open(output, 'w') { |file| file.write(output_string) }
