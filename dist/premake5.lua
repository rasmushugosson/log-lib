workspace "Game"
    architecture "x64"
    configurations { "Debug", "Release" }

    defines { "GLEW_STATIC" }

    filter "system:windows"
        defines { "AE_WINDOWS" }

    filter "system:macosx"
        defines { "AE_MACOS" }

    filter "system:linux"
        defines { "AE_LINUX" }

    filter "configurations:Debug"
        defines { "AE_DEBUG" }
        symbols "On"

    filter "configurations:Release"
        defines { "AE_RELEASE" }
        optimize "On"

    filter "action:vs*"
        startproject "Game"

project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}"

    files { "../engine/src/**.cpp", "../engine/src/**.h", "../engine/include/**.h", "../engine/vendor/**.cpp", "../engine/vendor/**.h" }

    includedirs { "../dep/GLFW/include", "../dep/GLEW/include", "../engine/include", "../engine/src", "../engine/vendor" }

    links { "opengl32.lib", "../dep/GLFW/lib/glfw3.lib", "../dep/GLEW/lib/glew32s.lib" }

    pchheader "general/pch.h"
    pchsource "../engine/src/general/pch.cpp" 

project "Game"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}"
  
    files { "../game/src/**.cpp", "../game/src/**.h" }
    includedirs { "../dep/GLFW/include", "../dep/GLEW/include", "../engine/include", "../game/src" }

    pchheader "general/pch.h"
    pchsource "../game/src/general/pch.cpp" 

    links { "opengl32.lib", "../dep/GLFW/lib/glfw3.lib", "../dep/GLEW/lib/glew32s.lib", "Engine" }
