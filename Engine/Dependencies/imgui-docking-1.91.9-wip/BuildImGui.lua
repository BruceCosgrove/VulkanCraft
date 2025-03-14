project "imgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "Off"

    prebuildcommands "%{RunPreBuild}"
    targetdir "%{TargetDir}"
    objdir "%{OBJDir}"

    files {
        "include/*.h",
        "include/*.cpp",
    }

    includedirs {
        "include",
        "%{IncludeDirs.engine}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.spdlog}",
    }

    filter "system:windows"
        systemversion "latest"
        usestandardpreprocessor "On"
        defines "ENG_SYSTEM_WINDOWS"

        files {
            "include/backends/imgui_impl_vulkan.cpp",
            "include/backends/imgui_impl_glfw.cpp",
        }

        includedirs {
            "%{IncludeDirs.vulkan}",
            "%{IncludeDirs.glfw}",
        }

        defines {
            "_CRT_SECURE_NO_WARNINGS",
        }

    filter "configurations:Debug"
        runtime "Debug"
        optimize "Debug"
        symbols "Full"

        defines {
            "ENG_CONFIG_DEBUG",
            "ENG_ENABLE_CONSOLE",
            "ENG_ENABLE_VERIFYS",
            "ENG_ENABLE_ASSERTS",
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"

        defines {
            "ENG_CONFIG_RELEASE",
            "ENG_ENABLE_CONSOLE",
            "ENG_ENABLE_VERIFYS",
        }

    filter "configurations:Dist"
        runtime "Release"
        optimize "Full"
        symbols "Off"

        defines {
            "ENG_CONFIG_DIST",
        }
