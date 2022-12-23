@echo off
start /B /W Premake/premake5 --file=independency.lua vs2022
rem Independency.sln
echo Build finished succesfully
pause