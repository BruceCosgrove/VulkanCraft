DependencyDir = "%{wks.location}/VulkanCraft/Dependencies/"
VulkanDir = os.getenv("VK_SDK_PATH") .. "/"

IncludeDirs = {}
LibraryDirs = {}
Libraries = {}

-- Include Directories
IncludeDirs["ctre"] = "%{DependencyDir}compile-time-regular-expressions-3.9.0/include/"
IncludeDirs["entt"] = "%{DependencyDir}entt-3.14.0/include/"
IncludeDirs["gcem"] = "%{DependencyDir}gcem-1.18.0/include/"
IncludeDirs["glfw"] = "%{DependencyDir}glfw-3.4/include/"
IncludeDirs["glm"] = "%{DependencyDir}glm-0.9.9.8/include/"
IncludeDirs["imgui"] = "%{DependencyDir}imgui-docking-1.91.9-wip/include/"
IncludeDirs["spdlog"] = "%{DependencyDir}spdlog-1.14.1/include/"
IncludeDirs["stb"] = "%{DependencyDir}stb-2.30/include/"
IncludeDirs["vulkan"] =  "%{VulkanDir}Include/"

-- Library Directories
LibraryDirs["vulkan"] = "%{VulkanDir}Lib/"

-- Libraries
Libraries["vulkan"] = "%{LibraryDirs.vulkan}vulkan-1.lib"
Libraries["shaderc_debug"] = "%{LibraryDirs.vulkan}shaderc_sharedd.lib"
Libraries["shaderc_release"] = "%{LibraryDirs.vulkan}shaderc_shared.lib"
