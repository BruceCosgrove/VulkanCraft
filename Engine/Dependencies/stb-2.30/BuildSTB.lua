project "stb"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "On"
    
    prebuildcommands "%{RunPreBuild}"
    targetdir "%{TargetDir}"
    objdir "%{OBJDir}"

    files {
        "include/stb_image.h",
        "include/stb_image_write.h",
        "src/stb.cpp",
    }

    includedirs {
        "include",
    }

    defines {
        "_CRT_SECURE_NO_WARNINGS",
    }

    filter "system:windows"
        systemversion "latest"

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
