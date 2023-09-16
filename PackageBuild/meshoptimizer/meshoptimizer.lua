require('../package_common')

ProjectName = 'MeshOptimizer'

SetupWorkspace(ProjectName)

project (ProjectName)

	files
	{
		'Source/src/**.cpp',
		'Source/src/**.hpp',
	}