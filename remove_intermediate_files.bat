@echo off

rem remove temp files in source folder
del source\*.user
del source\*.aps
del source\*.VC.db
rmdir /s /q source\ipch

rem remove temp files in build folder
for %%a in (build\Win32\Debug build\Win32\Release build\x64\Debug build\x64\Release) do (
	rmdir /s /q %%a\intermediate
	del %%a\*.ilk
	del %%a\*.pdb
	del %%a\*.iobj
	del %%a\*.ipdb
)