@echo off

rmdir /s /q intermediate

rem remove temp files in build folder
for %%a in (bin\vs2019\x86\Debug bin\vs2019\x86\Release bin\vs2019\x64\Debug bin\vs2019\x64\Release) do (
	del %%a\*.ilk
	del %%a\*.pdb
	del %%a\*.iobj
	del %%a\*.ipdb
)

for %%a in (bin\vs2022\x86\Debug bin\vs2022\x86\Release bin\vs2022\x64\Debug bin\vs2022\x64\Release) do (
	del %%a\*.ilk
	del %%a\*.pdb
	del %%a\*.iobj
	del %%a\*.ipdb
)

for %%a in (bin\vs2026\x86\Debug bin\vs2026\x86\Release bin\vs2026\x64\Debug bin\vs2026\x64\Release) do (
	del %%a\*.ilk
	del %%a\*.pdb
	del %%a\*.iobj
	del %%a\*.ipdb
)