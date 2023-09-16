require('../package_common')

ProjectName = 'EASTL'

SetupWorkspace(ProjectName)
	
project (ProjectName)

	defines
	{ 	
		'EA_PRAGMA_ONCE_SUPPORTED', 
		'EASTL_EASTDC_VSNPRINTF=1', 
		'EA_COMPILER_CPP11_ENABLED',
		'EASTL_MOVE_SEMANTICS_ENABLED',
		'EASTL_EXCEPTIONS_ENABLED=0',
	}
	
	files
	{
		'Source/source/**.cpp', 'Source/source/**.h',
		'Source/include/**.cpp', 'Source/include/**.h',
		'Source/test/packages/EAStdC/source/*.cpp' -- include to get portable sprintf
	}
		
	removefiles { 'Source/source/numeric_limits.cpp' } -- Empty under most defines
		
	externalincludedirs
	{
		'Source/include',
		'Source/test/packages/EAStdC/include',
		'Source/test/packages/EAAssert/include',
		'Source/test/packages/EABase/include/Common',
		'Source/test/packages/EAThread/include',
	}