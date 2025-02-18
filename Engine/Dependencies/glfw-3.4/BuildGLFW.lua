project "glfw"
    kind "StaticLib"
    language "C"
    cdialect "C17"
    staticruntime "On"
    
    prebuildcommands "%{RunPreBuild}"
    targetdir "%{TargetDir}"
    objdir "%{OBJDir}"

    -- https://stackoverflow.com/questions/70199224/trouble-linking-with-glfw-using-premake-and-vs2019
    files {
        "include/glfw/glfw3.h",
        "include/glfw/glfw3native.h",
        "src/context.c",
        "src/glfw_config.h",
        "src/init.c",
        "src/input.c",
        "src/internal.h",
        "src/mappings.h",
        "src/monitor.c",
        "src/null_init.c",
        "src/null_joystick.c",
        "src/null_joystick.h",
        "src/null_monitor.c",
        "src/null_platform.h",
        "src/null_window.c",
        "src/platform.c",
        "src/platform.h",
        "src/vulkan.c",
        "src/window.c",
    }

    filter "system:windows"
        systemversion "latest"

        files {
            "src/egl_context.c",
            "src/osmesa_context.c",
            "src/wgl_context.c",
            "src/win32_init.c",
            "src/win32_joystick.c",
            "src/win32_joystick.h",
            "src/win32_module.c",
            "src/win32_monitor.c",
            "src/win32_platform.h",
            "src/win32_thread.c",
            "src/win32_thread.h",
            "src/win32_time.c",
            "src/win32_time.h",
            "src/win32_window.c",
        }

        defines {
            "_GLFW_WIN32",
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
