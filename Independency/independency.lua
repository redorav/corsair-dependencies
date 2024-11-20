Workspace = "workspace/".._ACTION

-- Compilers
PlatformMSVC64	= "MSVC 64"
PlatformMSVC32	= "MSVC 32"
PlatformLLVM64	= "LLVM 64"
PlatformLLVM32	= "LLVM 32"

PlatformOSX64	= "OSX 64"

PlatformLinux64_GCC		= "Linux64_GCC"
PlatformLinux64_Clang	= "Linux64_Clang"

-- Directories
srcDir = "src"
externalDir = "external"

workspace "Independency"
	configurations { "Debug", "Release" }
	location (Workspace)
	cppdialect("c++17")
	
	includedirs
	{
		srcDir,
	}
		
	if(_ACTION == "xcode4") then
	
		platforms { PlatformOSX64 }
		toolset("clang")
		architecture("x64")
		buildoptions { "-msse4.1 -Wno-unused-variable" }
		linkoptions { "-stdlib=libc++" }
		
	elseif(_ACTION == "gmake") then
	
		platforms { PlatformLinux64_GCC, PlatformLinux64_Clang }
		architecture("x64")
		buildoptions { "-msse4.1 -Wno-unused-variable" }
		
		filter { "platforms:"..PlatformLinux64_GCC }
			toolset("gcc")
		
		filter { "platforms:"..PlatformLinux64_Clang }
			toolset("clang")
		
	else
	
		platforms { PlatformMSVC64 }
	
		--links("urlmon")
	
		filter { "platforms:"..PlatformMSVC64 }
			toolset("msc")
			architecture("x64")
	
	end
	
	filter { "configurations:Debug" }
		defines { "DEBUG" }
		symbols "on"
		inlining("auto") -- hlslpp relies on inlining for speed, big gains in debug builds without losing legibility
		optimize("debug")
		
	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "on"
		inlining("auto")
		optimize("full")

	filter{}

project "Independency"
	kind("ConsoleApp")
	language("C++")
	debugdir("")
	
	files
	{
		externalDir.."/argh/**.h",
		externalDir.."/pugixml/src/**.cpp",
		srcDir.."/**.h",
		srcDir.."/**.cpp"
	}
	
	defines 
	{
		"_CRT_NONSTDC_NO_DEPRECATE",
		"_CRT_SECURE_NO_WARNINGS",
		"_CRT_SECURE_NO_DEPRECATE",
	}
	
	includedirs
	{
		externalDir.."/pugixml/src",
		externalDir.."/subprocess.h",
		externalDir.."/argh",
	}