require('../package_common')

ProjectName = 'Wuffs'

SetupWorkspace('Wuffs')

project ('Wuffs')

	defines { 'WUFFS_IMPLEMENTATION' }
	vectorextensions ('AVX')

	files
	{
		'Source/wuffs-v0.3.c'
	}