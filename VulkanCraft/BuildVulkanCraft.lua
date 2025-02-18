project "VulkanCraft"
    language "C++"
    cppdialect "C++23"
    staticruntime "On"

    prebuildcommands "%{RunPreBuild}"
    targetdir "%{TargetDir}"
    objdir "%{OBJDir}"

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
        "Engine",
        "%{Libraries.vulkan}", -- Link this here to avoid a duplicate symbol warning.
    }

    filter "system:windows"
        systemversion "latest"
        usestandardpreprocessor "On"
        defines "ENG_SYSTEM_WINDOWS"

    filter "configurations:Debug"
        runtime "Debug"
        optimize "Debug"
        symbols "Full"
        kind "ConsoleApp"

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
        kind "ConsoleApp"

        defines {
            "ENG_CONFIG_RELEASE",
            "ENG_ENABLE_CONSOLE",
            "ENG_ENABLE_VERIFYS",
        }

    filter "configurations:Dist"
        runtime "Release"
        optimize "Full"
        symbols "Off"
        kind "WindowedApp"

        defines {
            "ENG_CONFIG_DIST",
        }
