require('../package_common')

ProjectName = 'MikkTSpace'

SetupWorkspace(ProjectName)

project (ProjectName)

	files
	{
		'Source/**.c',
		'Source/**.h',
	}