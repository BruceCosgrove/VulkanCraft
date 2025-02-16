DependencyDir = "%{wks.location}/__PROJECT_NAME__/Dependencies/"

IncludeDirs = {}
LibraryDirs = {}
Libraries = {}

-- Include Directories
	--	IncludeDirs["__EXAMPLE_PROJECT_NAME__"] = "%{__EXAMPLE_PROJECT_DIR__}/__EXAMPLE_PROJECT_INCLUDE_PATH__"

	-- Header and Source; they have their own project.

	-- Header-Only; they don't have their own project.

	-- Header and Lib; they also don't have their own project, but they do have linkable binaries.

--	LibraryDirs["__EXAMPLE_LIBRARY_NAME__"] = "%{__EXAMPLE_LIBRARY_DIR__}/__EXAMPLE_LIBRARY_DIR_PATH__"

--	Libraries["__EXAMPLE_LIBRARY_NAME__"] = "%{LibraryDirs.__EXAMPLE_LIBRARY_NAME__}/__EXAMPLE_LIBRARY_PATH__"
