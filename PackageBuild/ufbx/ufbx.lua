ProjectName = "Ufbx"

workspace(ProjectName)
	configurations { "Debug", "Release" }
	location ("Build/".._ACTION)
	architecture("x64")
	cppdialect("c++17")

	editandcontinue("off")
	
	filter ('configurations:Debug')
		defines { "DEBUG" }
		debugformat("c7") -- Do not create pdbs, instead store in lib
		symbols ("on")

	filter ('configurations:Release')
		defines { "NDEBUG" }
		optimize ("speed")
		symbols("off")
	
project (ProjectName)
	kind("StaticLib")
	language("C++")
	
	targetdir("Libraries/")
	targetname("%{wks.name}.".._ACTION..".%{cfg.buildcfg:lower()}")

	files
	{
		"Source/ufbx.c",
		"Source/ufbx.h",
	}

	filter { "configurations:*" } -- Workaround for MacOS nil in cfg