include "BuildExtensions.lua"
include "BuildDependencies.lua"

workspace "VulkanCraft"
    architecture "x86_64"
    startproject "VulkanCraft"
    configurations { "Debug", "Release", "Dist" }
    flags { "MultiProcessorCompile" }

    folder { "filter:Solution Items",
        -- Visual Studio
        ".editorconfig",

        -- Git
        ".gitignore",
        ".gitattributes",

        -- Misc
        "README.md",
    }

    folder { "filter:Build Scripts",
        -- Scripts
        "Scripts/GenerateProjects.bat",
        "Scripts/PreBuild.bat",

        -- Lua Scripts
        "BuildAll.lua",
        "BuildDependencies.lua",
        "BuildExtensions.lua",

        -- Project Build Scripts
        "Engine/BuildEngine.lua",
        "VulkanCraft/BuildVulkanCraft.lua",

        -- Dependency Project Build Scripts
        "Engine/Dependencies/glfw-3.4/BuildGLFW.lua",
        "Engine/Dependencies/imgui-docking-1.91.9-wip/BuildImGui.lua",
        "Engine/Dependencies/stb-2.30/BuildSTB.lua",
    }

RunPreBuild = "pushd \"%{wks.location}\" && \"Scripts/PreBuild.bat\" && popd"
TargetDir = "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"
OBJDir = "%{wks.location}/bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"

-- Add any projects here with 'include "Build__EXAMPLE_PROJECT_NAME__.lua"'
group "Dependencies"
    include "Engine/Dependencies/glfw-3.4/BuildGLFW.lua"
    include "Engine/Dependencies/imgui-docking-1.91.9-wip/BuildImGui.lua"
    include "Engine/Dependencies/stb-2.30/BuildSTB.lua"
group ""
include "Engine/BuildEngine.lua"
include "VulkanCraft/BuildVulkanCraft.lua"
