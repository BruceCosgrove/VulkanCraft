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
        "VulkanCraft/BuildVulkanCraft.lua",

        -- Dependency Project Build Scripts
        "VulkanCraft/Dependencies/glfw-3.4/BuildGLFW.lua",
        "VulkanCraft/Dependencies/imgui-docking-1.91.9-wip/BuildImGui.lua",
        "VulkanCraft/Dependencies/stb-2.30/BuildSTB.lua",
    }

RunPreBuild = "pushd \"%{wks.location}\" && \"Scripts/PreBuild.bat\" && popd"
TargetDir = "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"
OBJDir = "%{wks.location}/bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"

-- Add any projects here with 'include "Build__EXAMPLE_PROJECT_NAME__.lua"'
group "Dependencies"
    include "VulkanCraft/Dependencies/glfw-3.4/BuildGLFW.lua"
    include "VulkanCraft/Dependencies/imgui-docking-1.91.9-wip/BuildImGui.lua"
    include "VulkanCraft/Dependencies/stb-2.30/BuildSTB.lua"
group ""
include "VulkanCraft/BuildVulkanCraft.lua"
