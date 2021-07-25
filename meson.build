project('swan', 'cpp',
	default_options: ['cpp_std=c++2a'])

libdl = meson.get_compiler('cpp').find_library('dl')
libz = meson.get_compiler('cpp').find_library('z')
libgles2 = meson.get_compiler('cpp').find_library('GLESv2')
libsdl2 = dependency('sdl2')
libsdl2_image = dependency('SDL2_image')
libthreads = dependency('threads')

subdir('third-party')

common = declare_dependency(
	include_directories: 'include',
	dependencies: libmsgpack)

subdir('libcygnet')
subdir('libswan')
subdir('core.mod')
executable('swan', 'src/main.cc',
	install: true,
	install_rpath: get_option('libdir'),
	dependencies: [libswan, libcygnet, common, libimgui, libsdl2, libsdl2_image])

install_subdir('assets', install_dir: '')
