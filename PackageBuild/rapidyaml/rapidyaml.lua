ProjectName = "RapidYAML"

workspace(ProjectName)
	configurations { "Debug", "Release" }
	location ("Build/".._ACTION)
	architecture("x64")
	cppdialect("c++17")
	
	if _ACTION == "vs2017" then
		toolset("msc") -- Use default VS toolset TODO do this platform specific
		--toolset("msc-llvm-vs2014") -- Use for Clang on VS
	end

	editandcontinue("off")
	
	configuration "Debug"
		defines { "DEBUG" }
		debugformat("c7") -- Do not create pdbs, instead store in lib
		symbols ("on")

	configuration "Release"
		defines { "NDEBUG" }
		optimize ("speed")
		symbols("off")
	
project (ProjectName)
	kind("StaticLib")
	language("C++")
	
	targetdir("Libraries/")
	targetname("%{wks.name}.".._ACTION..".%{cfg.buildcfg:lower()}")
	
	defines { "RYML_USE_ASSERT=0" }

	files
	{
		"Source/src/**.cpp",
		"Source/src/**.hpp",
		"Source/ext/c4core/src/c4/*.cpp",
		"Source/ext/c4core/src/c4/*.hpp"
	}
	
	sysincludedirs
	{
		"Source/src",
		"Source/ext/c4core/src"
	}

	filter { "configurations:*" } -- Workaround for MacOS nil in cfg