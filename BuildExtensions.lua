require('vstudio')

local p = premake
local vc = p.vstudio.vc2010
local sln = p.vstudio.sln2005

p.api.register {
	name = "usestdpreproc",
	scope = "config",
	kind = "boolean",
}

p.api.register {
	name = "enablemodules",
	scope = "config",
	kind = "boolean",
}

-- Usages:
--	Add paths to files (may be in many different directories) under the filter named __FILTER_NAME__.
--		folder { "filter:__FILTER_NAME__",
--			"__FILE_NAME_1__",
--			"__FILE_NAME_2__",
--			...
--			"__FILE_NAME_N__",
--		}
--	Add all existing files under the folder named __EXISTING_FOLDER_NAME__.
--		folder { "existing:__EXISTING_FOLDER_NAME__" }
p.api.register {
	name = "folder",
	scope = "workspace",
	kind = "list:string",
}

p.override(vc.elements, "clCompile", function(base, cfg)
	local calls = base(cfg)

	table.replace(calls, vc.floatingPointModel, function(cfg)
		if cfg.floatingpoint and cfg.floatingpoint ~= "Default" then
			if cfg.floatingpoint ~= "None" then
				vc.element("FloatingPointModel", nil, cfg.floatingpoint)
			else
				p.w("<FloatingPointModel />") -- Having "None" as the value causes visual studio's build to crash.
			end
		end
	end)

	table.insertafter(calls, vc.languageStandardC, function(cfg)
		if _ACTION >= "vs2019" and cfg.usestdpreproc ~= nil then
			vc.element("UseStandardPreprocessor", nil, iif(cfg.usestdpreproc, "true", "false"))
		end
	end)

	table.insertafter(calls, vc.scanSourceForModuleDependencies, function(cfg)
		if _ACTION >= "vs2022" and cfg.enablemodules ~= nil then
			vc.element("EnableModules", nil, iif(cfg.enablemodules, "true", "false"))
			-- Visual Studio added a default-on setting that builds stl modules, and it tries to use a project's pch, which makes compilation fail.
			vc.element("BuildStlModules", nil, iif(cfg.enablemodules, "true", "false"))
		end
	end)

	return calls
end)

p.override(sln, "projects", function(base, wks)
	if wks.folder and #wks.folder > 0 then
		local solutionFolderGUID = "{2150E333-8FDC-42A3-9474-1A3956D46DE8}" -- See https://www.codeproject.com/Reference/720512/List-of-Visual-Studio-Project-Type-GUIDs

		-- i is the index (starting at 1), path is the element at said index
		for i, path in ipairs(wks.folder) do
			local _, colonIndex = path:find(":", 1, true)
			if colonIndex then
				if i > 1 then
					p.pop("EndProjectSection")
					p.pop("EndProject")
				end

				local folderKind = path:sub(1, colonIndex - 1)
				local folderName = path:sub(colonIndex + 1)
				
				p.push("Project(\"" .. solutionFolderGUID .. "\") = \"" .. folderName .. "\", \"" .. folderName .. "\", \"{" .. os.uuid(folderName .. wks.name) .. "}\"")
				p.push("ProjectSection(SolutionItems) = preProject")

				-- add all existing files in a directory
				if folderKind == "existing" then
					-- https://www.reddit.com/r/lua/comments/xpqabs/how_to_list_all_files_in_a_directory/
					for dir in io.popen("dir \"" .. folderName .. "\" /b"):lines() do
						local subpath = folderName .. "\\" .. dir
						p.w(subpath .. " = " .. subpath)
					end
				end

			else
				p.w(path .. " = " .. path)
			end
		end

		p.pop("EndProjectSection")
		p.pop("EndProject")
	end
	base(wks)
end)
