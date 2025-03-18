EngineDir = "%{wks.location}/Engine/"
EngineDependencyDir = "%{EngineDir}Dependencies/"
VulkanDir = os.getenv("VK_SDK_PATH") .. "/"

IncludeDirs = {}
LibraryDirs = {}
Libraries = {}

-- Include Directories
IncludeDirs["engine"] = "%{EngineDir}Source/"
IncludeDirs["glfw"] = "%{EngineDependencyDir}glfw-3.4/include/"
IncludeDirs["glm"] = "%{EngineDependencyDir}glm-0.9.9.8/include/"
IncludeDirs["spdlog"] = "%{EngineDependencyDir}spdlog-1.14.1/include/"
IncludeDirs["stb"] = "%{EngineDependencyDir}stb-2.30/include/"
IncludeDirs["vulkan"] =  "%{VulkanDir}Include/"

-- Library Directories
LibraryDirs["vulkan"] = "%{VulkanDir}Lib/"

-- Libraries
Libraries["vulkan"] = "%{LibraryDirs.vulkan}vulkan-1.lib"

Libraries["shaderc_debug"] = "%{LibraryDirs.vulkan}shaderc_sharedd.lib"
Libraries["spirv_cross_debug"] = "%{LibraryDirs.vulkan}spirv-cross-cored.lib"
Libraries["spirv_cross_glsl_debug"] = "%{LibraryDirs.vulkan}spirv-cross-glsld.lib"
Libraries["spirv_cross_reflect_debug"] = "%{LibraryDirs.vulkan}spirv-cross-reflectd.lib"

Libraries["shaderc_release"] = "%{LibraryDirs.vulkan}shaderc_shared.lib"
Libraries["spirv_cross_release"] = "%{LibraryDirs.vulkan}spirv-cross-core.lib"
Libraries["spirv_cross_glsl_release"] = "%{LibraryDirs.vulkan}spirv-cross-glsl.lib"
Libraries["spirv_cross_reflect_release"] = "%{LibraryDirs.vulkan}spirv-cross-reflect.lib"
