@echo on
"%VS160COMNTOOLS%..\IDE\devenv.com" make\vs2019\QuadCollapseLOD.sln /build "Debug|Win32"
"%VS160COMNTOOLS%..\IDE\devenv.com" make\vs2019\QuadCollapseLOD.sln /build "Release|Win32"
"%VS160COMNTOOLS%..\IDE\devenv.com" make\vs2019\QuadCollapseLOD.sln /build "Debug|x64"
"%VS160COMNTOOLS%..\IDE\devenv.com" make\vs2019\QuadCollapseLOD.sln /build "Release|x64"
pause