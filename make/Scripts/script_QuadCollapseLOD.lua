-- 2022.06.14 Tue.

require ("script_extension")
require ("script_configuration")

--------------------------------------------------------------------------------
-- quake2 workspace --
--------------------------------------------------------------------------------

if is_linux then

	workspace "%{WorkspaceName}"
		configurations { "debug", "release" }
		platforms { "x86", "x64" }
		location "../%{_ACTION}"
		
		filter "configurations:debug"
		
			defines { "_DEBUG" }
			symbols "On"
				
		filter "configurations:release"
		
			defines { "NDEBUG" }
			symbols "On"
			optimize "Speed"
			
		filter {} -- close filter scope
	
else

	workspace "%{WorkspaceName}"
		configurations { "debug", "release" }
		platforms { "x86", "x64" }
		location "../%{_ACTION}"
			
        filter { "platforms:x86" }
	        system "Windows"
	        architecture "x86"
        
		filter { "platforms:x64" }
			system "Windows"
			architecture "x86_64"
			
		filter "configurations:debug"
		
			defines { "_DEBUG" }
			symbols "On"
				
		filter "configurations:release"
		
			defines { "NDEBUG" }
			symbols "On"
			optimize "Speed"
			
		filter {} -- close filter scope

end

--------------------------------------------------------------------------------
-- projects --
--------------------------------------------------------------------------------

require ("script_proj_QuadCollapseLOD")

