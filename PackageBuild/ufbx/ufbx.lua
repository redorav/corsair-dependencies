require('../package_common')

ProjectName = 'Ufbx'

SetupWorkspace('Ufbx')

project ('Ufbx')

	files
	{
		'Source/ufbx.c',
		'Source/ufbx.h',
	}