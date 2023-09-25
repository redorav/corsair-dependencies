require('../package_common')

ProjectName = 'Stb'

SetupWorkspace('Stb')

project ('Stb')

	includedirs { 'Source/stb/' }
	
	files
	{
		'Source/stb_compile.cpp',
		'Source/stb/*.h',
	}