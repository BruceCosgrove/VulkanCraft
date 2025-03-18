project "yaml-cpp"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"
    staticruntime "Off"

    prebuildcommands "%{RunPreBuild}"
    targetdir "%{TargetDir}"
    objdir "%{OBJDir}"

    files {
        "include/yaml-cpp/**.h",
        "src/**.h",
        "src/**.cpp",
		"src/contrib/yaml-cpp.nativs",
    }

    includedirs {
        "include",
    }

	defines {
		"YAML_CPP_STATIC_DEFINE",
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
