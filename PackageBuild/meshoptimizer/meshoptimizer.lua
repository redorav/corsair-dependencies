require('../package_common')

ProjectName = 'MeshOptimizer'

SetupWorkspace(ProjectName)

project (ProjectName)

	defines { 'RYML_USE_ASSERT=0' }

	files
	{
		'Source/src/**.cpp',
		'Source/src/**.hpp',
	}