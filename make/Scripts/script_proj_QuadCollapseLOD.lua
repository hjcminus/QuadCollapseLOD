----------------------------------------
-- QuadCollapseLOD --
----------------------------------------

if is_linux then

	project "QuadCollapseLOD"
		location "../%{_ACTION}/projects"
		kind "ConsoleApp"
		language "C++"
		targetdir "../../%{TargetParentPath}/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}"
		objdir "../../%{ObjParentPath}/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}/QuadCollapseLOD"
		
		files {
			"../../source/*.h",
            "../../source/*.cpp"
		}
        
		includedirs {
			"../../external"
		}
		
		links {
			"GLEW",
			"GL",
			"glut",
			"m"
		}

else

	

end
