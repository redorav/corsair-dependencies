ProjectName = "EASTL"

workspace(ProjectName)
	configurations { "Debug", "Release" }
	location ("Build/".._ACTION)
	architecture("x64")
	cppdialect("c++17")

	if _ACTION == "vs2017" then
		toolset("msc") -- Use default VS toolset TODO do this platform specific
		--toolset("msc-llvm-vs2014") -- Use for Clang on VS
	end
	
	filter ('configurations:Debug')
		defines { "DEBUG", "_DEBUG" }
		debugformat("c7") -- Do not create pdbs, instead store in lib
		symbols "on"

	filter ('configurations:Release')
		defines { "NDEBUG" }
		optimize ("speed")
		symbols("off")
	
project (ProjectName)
	kind("StaticLib")
	language("C++")

	targetdir("Libraries/")
	targetname("%{wks.name}.".._ACTION..".%{cfg.buildcfg:lower()}")
	
	defines
	{ 	
		"EA_PRAGMA_ONCE_SUPPORTED", 
		"EASTL_EASTDC_VSNPRINTF=1", 
		"EA_COMPILER_CPP11_ENABLED",
		"EASTL_MOVE_SEMANTICS_ENABLED",
		"EASTL_EXCEPTIONS_ENABLED=0",
	}
	
	files
	{
		"Source/source/**.cpp", "Source/source/**.h",
		"Source/include/**.cpp", "Source/include/**.h",
		"Source/test/packages/EAStdC/source/*.cpp" -- include to get portable sprintf
	}
		
	removefiles { "Source/source/numeric_limits.cpp" } -- Empty under most defines
		
	sysincludedirs
	{
		"Source/include",
		"Source/test/packages/EAStdC/include",
		"Source/test/packages/EAAssert/include",
		"Source/test/packages/EABase/include/Common",
		"Source/test/packages/EAThread/include",
	}