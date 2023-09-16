require('../package_common')

ProjectName = 'RapidYAML'

SetupWorkspace(ProjectName)

project (ProjectName)

	defines { 'RYML_USE_ASSERT=0' }

	files
	{
		'Source/src/**.cpp',
		'Source/src/**.hpp',
		'Source/ext/c4core/src/c4/*.cpp',
		'Source/ext/c4core/src/c4/*.hpp'
	}
	
	sysincludedirs
	{
		'Source/src',
		'Source/ext/c4core/src'
	}