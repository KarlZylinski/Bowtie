solution "bowtie"
   language "C++"
   configurations { "debug", "release" }
   forceincludes { "$(BOWTIE_SOURCE)/base/base/types.h", "$(BOWTIE_SOURCE)/base/base/assert.h" }
   flags { "Unicode", "ExtraWarnings", "FatalWarnings" }
 
   configuration "debug"
      defines { "DEBUG" }
      flags { "Symbols" }
      architecture "x64"

   configuration "release"
      defines { "NDEBUG" }
      flags { "Optimize" }
      architecture "x64"

   platforms { "windows" }
   configuration { "windows" }
      defines { "WINDOWS" }
      buildoptions { "/wd4996", "/wd4238", "/TP" }
      linkoptions { "/ignore:4098" }
      flags { "StaticRuntime" }

   defines { "_CRT_SECURE_NO_WARNINGS" }

   project "application_windows"
      location "application_windows"
      kind "WindowedApp"
      files { "application_windows/**.h", "application_windows/**.cpp" }
      flags { "WinMain" }
      links { "base", "engine", "game", "opengl_renderer", "os", "lua51.lib", "opengl32", "dbghelp" }
      includedirs { "base", "engine", "game", "opengl_renderer", "os" }
      configuration "debug"
         libdirs { "$(BOWTIE_LIBS)/luajit/lib/debug" }
      configuration "release"
         libdirs { "$(BOWTIE_LIBS)/luajit/lib/release" }

   project "base"
      location "base"
      kind "StaticLib"
      files { "base/**.c", "base/**.h", "base/**.cpp" }

   project "engine"
      location "engine"
      kind "StaticLib"
      files { "engine/**.h", "engine/**.cpp" }
      includedirs { "base", "game", "os" }

   project "game"
      location "game"
      kind "StaticLib"
      files { "game/**.h", "game/**.cpp" }
      includedirs { "base", "game", "engine", "$(BOWTIE_LIBS)/luajit/include", "os" }

   project "opengl_renderer"
      location "opengl_renderer"
      kind "StaticLib"
      files { "opengl_renderer/**.h", "opengl_renderer/**.cpp", "opengl_renderer/**.c" }
      includedirs { "base", "renderer", "engine" }
      defines { "GLEW_STATIC" }

   project "os"
      location "os"
      kind "StaticLib"
      files { "os/**.h", "os/**.cpp" }
      includedirs { "base" }
      