include "BuildExtensions.lua"

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
        "BuildExtensions.lua",

        -- Project Build Scripts
        "Engine/BuildEngine.lua",
        "Engine/EngineDependencies.lua",
        "VulkanCraft/BuildVulkanCraft.lua",
        "VulkanCraft/VulkanCraftDependencies.lua",

        -- Dependency Project Build Scripts
        "Engine/Dependencies/glfw-3.4/BuildGLFW.lua",
        "Engine/Dependencies/stb-2.30/BuildSTB.lua",
        "VulkanCraft/Dependencies/imgui-docking-1.91.9-wip/BuildImGui.lua",
        "VulkanCraft/Dependencies/yaml-cpp-0.8.0/BuildYamlCPP.lua",
    }

RunPreBuild = "pushd \"%{wks.location}\" && \"Scripts/PreBuild.bat\" && popd"
TargetDir = "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"
OBJDir = "%{wks.location}/bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"

include "Engine/EngineDependencies.lua"
include "VulkanCraft/VulkanCraftDependencies.lua"

-- Dependency Projects
group "Dependencies"
    include "Engine/Dependencies/glfw-3.4/BuildGLFW.lua"
    include "Engine/Dependencies/stb-2.30/BuildSTB.lua"
    include "VulkanCraft/Dependencies/imgui-docking-1.91.9-wip/BuildImGui.lua"
    include "VulkanCraft/Dependencies/yaml-cpp-0.8.0/BuildYamlCPP.lua"
group ""
include "Engine/BuildEngine.lua"
include "VulkanCraft/BuildVulkanCraft.lua"
