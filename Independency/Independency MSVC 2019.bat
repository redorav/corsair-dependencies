@echo off
start /B /W Premake/premake5 --file=independency.lua vs2019
rem Independency.sln
echo Build finished succesfully
pause