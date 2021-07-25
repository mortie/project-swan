libcygnet = declare_dependency(
	include_directories: 'include',
	link_with: library('cygnet',
		'src/GlWrappers.cc',
		'src/Renderer.cc',
		'src/ResourceManager.cc',
		'src/shaders.cc',
		'src/TileAtlas.cc',
		'src/util.cc',
		'src/Window.cc',
		dependencies: [common, libtracy, libthreads, libsdl2, libgles2],
		install: true,
		include_directories: 'include/cygnet'))
