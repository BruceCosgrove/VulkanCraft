project "VulkanCraft"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
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
        -- Add any project source directories here.
        "Source",

        -- Add any dependency includes here.
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
        -- Add any dependency libs via their project names here.
        "glfw",
        "imgui",
        "stb",
        "%{Libraries.vulkan}",
    }

    filter "system:windows"
        systemversion "latest"
        usestdpreproc "On" -- msvc doesn't provide __VA_OPT__ by default; this fixes that.
        defines "SYSTEM_WINDOWS"

    filter "configurations:Debug"
        runtime "Debug"
        optimize "Debug"
        symbols "Full"
        defines "CONFIG_DEBUG"

        links {
            "%{Libraries.shaderc_debug}",
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"
        defines "CONFIG_RELEASE"

        links {
            "%{Libraries.shaderc_release}",
        }

    filter "configurations:Dist"
        kind "WindowedApp"
        runtime "Release"
        optimize "Full"
        symbols "Off"
        defines "CONFIG_DIST"

        links {
            "%{Libraries.shaderc_release}",
        }
