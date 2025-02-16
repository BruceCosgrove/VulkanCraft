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
		--	"__EXAMPLE_PROJECT_NAME__/Dependencies/__EXAMPLE_DEPENDENCY_NAME__/Build__EXAMPLE_DEPENDENCY_NAME__.lua",
	}

RunPreBuild = "pushd \"%{wks.location}\" && \"Scripts/PreBuild.bat\" && popd"
TargetDir = "%{wks.location}/bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"
OBJDir = "%{wks.location}/bin-int/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"

-- Add any projects here with 'include "Build__EXAMPLE_PROJECT_NAME__.lua"'
include "VulkanCraft/BuildVulkanCraft.lua"
