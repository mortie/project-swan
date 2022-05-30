libcygnet = declare_dependency(
	include_directories: 'include',
	link_with: library('cygnet',
		'src/gl.cc',
		'src/GlWrappers.cc',
		'src/Renderer.cc',
		'src/ResourceManager.cc',
		'src/shaders.cc',
		'src/TileAtlas.cc',
		'src/util.cc',
		dependencies: [common, libtracy, libthreads, libgl, libglfw3],
		install: true,
		include_directories: 'include/cygnet'))
