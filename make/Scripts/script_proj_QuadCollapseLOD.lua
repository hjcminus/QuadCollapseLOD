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

	project "QuadCollapseLOD"
		location "../%{_ACTION}/projects"
		kind "ConsoleApp"
		language "C++"
		targetdir "../../%{TargetParentPath}/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}"
		objdir "../../%{ObjParentPath}/%{_ACTION}/%{cfg.platform}/%{cfg.buildcfg}/QuadCollapseLOD"
		
        defines {
			"WIN32",
			"_CONSOLE"
		}
        
		files {
			"../../source/*.h",
            "../../source/*.cpp"
		}
        
		includedirs {
            "../../external",
			"../../external/glew-2.1.0/include",
            "../../external/freeglut/include"
		}
        
        libdirs { 
			"../../external/glew-2.1.0/lib/Release/%{cfg.platform}",
            "../../external/freeglut/lib/%{cfg.platform}"
		}
		
		links {
			"glew32.lib",
			"freeglut.lib"
		}

end
