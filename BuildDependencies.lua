DependencyDir = "%{wks.location}/VulkanCraft/Dependencies/"

IncludeDirs = {}
LibraryDirs = {}
Libraries = {}

-- Include Directories
    -- Header and Source; they have their own project.
    IncludeDirs["glfw"] = "%{DependencyDir}glfw-3.4/include/"
    IncludeDirs["stb"] = "%{DependencyDir}stb-2.30/include/"
    IncludeDirs["imgui"] = "%{DependencyDir}imgui-docking-1.91.9-wip/include/"

    -- Header-Only; they don't have their own project.
    IncludeDirs["ctre"] = "%{DependencyDir}compile-time-regular-expressions-3.9.0/include/"
    IncludeDirs["gcem"] = "%{DependencyDir}gcem-1.18.0/include/"
    IncludeDirs["glm"] = "%{DependencyDir}glm-0.9.9.8/include/"
    IncludeDirs["spdlog"] = "%{DependencyDir}spdlog-1.15.1/include/"

    -- Header and Lib; they also don't have their own project, but they do have linkable binaries.

--	LibraryDirs["__EXAMPLE_LIBRARY_NAME__"] = "%{__EXAMPLE_LIBRARY_DIR__}/__EXAMPLE_LIBRARY_DIR_PATH__"

--	Libraries["__EXAMPLE_LIBRARY_NAME__"] = "%{LibraryDirs.__EXAMPLE_LIBRARY_NAME__}/__EXAMPLE_LIBRARY_PATH__"
