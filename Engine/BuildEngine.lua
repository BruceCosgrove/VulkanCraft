project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"

    prebuildcommands "%{RunPreBuild}"
    targetdir "%{TargetDir}"
    objdir "%{OBJDir}"

    defines {
        "ENG_ENGINE",
    }

    files {
        "Source/**.h",
        "Source/**.c",
        "Source/**.hpp",
        "Source/**.cpp",
        "Source/**.inl",
        "Source/**.ixx",
    }

    includedirs {
        "%{IncludeDirs.engine}",
        "%{IncludeDirs.ctre}",
        "%{IncludeDirs.entt}",
        "%{IncludeDirs.gcem}",
        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
        "%{IncludeDirs.imgui}",
        "%{IncludeDirs.spdlog}",
        "%{IncludeDirs.stb}",
        "%{IncludeDirs.vulkan}",
    }

    links {
        "glfw",
        "imgui",
        "stb",
    }

    filter "system:windows"
        systemversion "latest"
        usestdpreproc "On" -- msvc doesn't provide __VA_OPT__ by default; this fixes that.
        defines "ENG_SYSTEM_WINDOWS"

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

        links {
            "%{Libraries.shaderc_debug}",
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

        links {
            "%{Libraries.shaderc_release}",
        }

    filter "configurations:Dist"
        runtime "Release"
        optimize "Full"
        symbols "Off"

        defines {
            "ENG_CONFIG_DIST",
        }

        links {
            "%{Libraries.shaderc_release}",
        }
