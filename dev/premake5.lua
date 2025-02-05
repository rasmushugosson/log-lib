workspace "Log"
    architecture "x64"
    configurations { "Debug", "Release" }

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
        startproject "Sandbox"

project "Log"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}"

    files { "../log-lib/src/**.cpp", "../log-lib/src/**.h", "../log-lib/include/**.h" }

    includedirs { "../log-lib/include", "../log-lib/src" }

    pchheader "general/pch.h"
    pchsource "../log-lib/src/general/pch.cpp" 

project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    objdir "obj/%{cfg.buildcfg}"
  
    files { "../sandbox/src/**.cpp", "../sandbox/src/**.h" }
    includedirs { "../log-lib/include", "../sandbox/src" }

    links { "Log" }
