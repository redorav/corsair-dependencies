require('../package_common')

ProjectName = 'Glslang'

SetupWorkspace(ProjectName)

project (ProjectName)

	files
	{
		'Source/glslang/GenericCodeGen/**.cpp', 'Source/glslang/GenericCodeGen/**.h',
		'Source/glslang/Include/**.cpp', 'Source/glslang/Include/**.h',
		'Source/glslang/MachineIndependent/**.cpp', 'Source/glslang/MachineIndependent/**.h',
		'Source/glslang/HLSL/**.cpp', 'Source/glslang/HLSL/**.h',
		'Source/glslang/Public/**.cpp', 'Source/glslang/Public/**.h',
		'Source/glslang/OSDependent/*.h',
		'Source/hlsl/**.cpp', 'Source/hlsl/**.h',
		'Source/SPIRV/**.cpp', 'Source/SPIRV/**.h',
		'Source/OGLCompilersDLL/**.cpp', 'Source/OGLCompilersDLL/**.h',
	}	
			
	sysincludedirs
	{
		'Source/', 
		'Source/OGLCompilersDLL'
	}
	
	filter('system:windows')
		files { 'Source/glslang/OSDependent/windows/**.cpp', 'Source/glslang/OSDependent/windows/**.h' }
	
	filter('system:not windows')
		files { 'Source/glslang/OSDependent/unix/**.cpp', 'Source/glslang/OSDependent/unix/**.h' }
	
	filter {}
	
	defines { 'ENABLE_HLSL' }