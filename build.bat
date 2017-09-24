@echo on
"%VS140COMNTOOLS%..\IDE\devenv.com" source\QuadCollapseLOD.sln /build "Debug|x86"
"%VS140COMNTOOLS%..\IDE\devenv.com" source\QuadCollapseLOD.sln /build "Release|x86"
"%VS140COMNTOOLS%..\IDE\devenv.com" source\QuadCollapseLOD.sln /build "Debug|x64"
"%VS140COMNTOOLS%..\IDE\devenv.com" source\QuadCollapseLOD.sln /build "Release|x64"
pause