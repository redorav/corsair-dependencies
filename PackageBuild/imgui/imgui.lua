require('../package_common')

ProjectName = 'ImGui'

SetupWorkspace(ProjectName)

project (ProjectName)

	files
	{
		'Source/*.cpp',
		'Source/*.hpp',
		'Source/*.h',
		'Source/misc/freetype/*.cpp',
		'Source/misc/freetype/*.h',
	}
	
	includedirs { 'Source', 'Source/freetype-2.13.3/include' }
	
	removefiles {}
	
	defines { 'IMGUI_DISABLE_OBSOLETE_FUNCTIONS' }
	
	filter { 'configurations:*' } -- Workaround for MacOS nil in cfg