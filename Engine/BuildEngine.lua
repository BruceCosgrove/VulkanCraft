project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++23"
    staticruntime "Off"

    prebuildcommands "%{RunPreBuild}"
    targetdir "%{TargetDir}"
    objdir "%{OBJDir}"

    defines {
        "ENG_ENGINE",
        "GLFW_INCLUDE_NONE",
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",
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
        "%{IncludeDirs.glfw}",
        "%{IncludeDirs.glm}",
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
        usestandardpreprocessor "On"
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
            "%{Libraries.spirv_cross_debug}",
            "%{Libraries.spirv_cross_glsl_debug}",
            "%{Libraries.spirv_cross_reflect_debug}",
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
            "%{Libraries.spirv_cross_release}",
            "%{Libraries.spirv_cross_glsl_release}",
            "%{Libraries.spirv_cross_reflect_release}",
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
            "%{Libraries.spirv_cross_release}",
            "%{Libraries.spirv_cross_glsl_release}",
            "%{Libraries.spirv_cross_reflect_release}",
        }
