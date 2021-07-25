libcpptoml = declare_dependency(
	include_directories: 'cpptoml/include')

libfmt = declare_dependency(
	include_directories: 'fmt/include',
	link_with: library('fmt',
		'fmt/src/format.cc',
		'fmt/src/os.cc',
		include_directories: 'fmt/include'))

libimgui = declare_dependency(
	include_directories: ['imgui', 'imgui-plot/include', 'imgui_sdl'],
	link_with: library('imgui',
		'imgui/imgui_demo.cpp',
		'imgui/imgui_draw.cpp',
		'imgui/imgui_widgets.cpp',
		'imgui/imgui.cpp',
		'imgui_sdl/imgui_sdl.cpp',
		'imgui-plot/src/imgui_plot.cpp',
		include_directories: ['imgui', 'imgui-plot/include', 'imgui_sdl'],
		dependencies: libsdl2,
		cpp_args: ['-Wno-sign-compare', '-Wno-deprecated-enum-enum-conversion']))

libmsgpack = declare_dependency(
	include_directories: 'msgpack-c/include')

libperlinnoise = declare_dependency(
	include_directories: 'PerlinNoise')

libtracy = declare_dependency(
	include_directories: 'tracy',
	link_with: library('tracy',
		'tracy/TracyClient.cpp',
		dependencies: libthreads))
