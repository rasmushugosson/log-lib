-- Reusable Log project definition
-- Can be included by parent projects via: include("path/to/log-lib/log-project.lua")

-- Guard to prevent multiple inclusions
if LOG_LIB_INCLUDED then
    return
end
LOG_LIB_INCLUDED = true

local log_lib_dir = path.getdirectory(_SCRIPT)
local log_lib_src = log_lib_dir .. "/log-lib"

-- Export include path for dependent projects
LOG_LIB_INCLUDE_DIR = log_lib_src .. "/include"

project("Log")
    kind("StaticLib")
    language("C++")
    cppdialect("C++23")
    objdir("obj/%{prj.name}/%{cfg.buildcfg}")
    targetdir("bin/%{prj.name}/%{cfg.buildcfg}")

    files({
        log_lib_src .. "/src/**.cpp",
        log_lib_src .. "/src/**.h",
        log_lib_src .. "/include/**.h"
    })

    includedirs({
        log_lib_src .. "/include",
        log_lib_src .. "/src"
    })

    pchheader(path.getabsolute(log_lib_src .. "/src/general/pch.h"))
    pchsource(log_lib_src .. "/src/general/pch.cpp")
