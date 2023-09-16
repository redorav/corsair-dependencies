require('../package_common')

ProjectName = 'SPIRV-Reflect'

SetupWorkspace(ProjectName)

project (ProjectName)

	files
	{
		'Source/spirv_reflect.c',
		'Source/spirv_reflect.h',
	}