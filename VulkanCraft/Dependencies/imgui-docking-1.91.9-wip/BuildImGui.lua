project "imgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"
    
    prebuildcommands "%{RunPreBuild}"
    targetdir "%{TargetDir}"
    objdir "%{OBJDir}"

    files {
        "include/*.h",
        "include/*.cpp",
    }

    includedirs {
        "include",
    }

    filter "system:windows"
        systemversion "latest"
        usestdpreproc "On" -- msvc doesn't provide __VA_OPT__ by default; this fixes that.

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

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        symbols "On"

    filter "configurations:Dist"
        runtime "Release"
        optimize "Full"
        symbols "Off"
