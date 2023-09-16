
function SetupWorkspace(SolutionName)

workspace(SolutionName)
	configurations { 'Debug', 'Release' }
	location ('Build/'.._ACTION)
	architecture('x64')
	
	kind('StaticLib')
	language('C++')
	cppdialect('c++17')

	editandcontinue('off')
	
	filter ("system:windows")
		toolset("msc") -- Use default VS toolset
		--toolset("msc-llvm-vs2014") -- Use for Clang on VS
		flags { "multiprocessorcompile" }
	
	targetdir('Libraries/')
	targetname('%{wks.name}.'.._ACTION..'.%{cfg.buildcfg:lower()}')

	filter ('configurations:Debug')
		defines { 'DEBUG' }
		debugformat('c7') -- Do not create pdbs, instead store in lib
		symbols ('on')

	filter ('configurations:Release')
		defines { 'NDEBUG' }
		optimize ('speed')
		symbols('off')

end