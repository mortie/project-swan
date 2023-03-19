project(
  'swan', 'cpp',
  default_options: ['cpp_std=c++2a'],
)

shaderpp_prog = find_program('./scripts/shaderpp.py')

tracy_proj = subproject('tracy')
libtracy = tracy_proj.get_variable('tracy_dep')
if get_option('trace')
  add_project_arguments('-DTRACY_ENABLE', language: 'cpp')
endif

libdl = meson.get_compiler('cpp').find_library('dl')
libz = meson.get_compiler('cpp').find_library('z')

libglfw3 = dependency('glfw3')
libgl = dependency('OpenGL')
#libgl = meson.get_compiler('cpp').find_library('GLESv2')

libsdl2_image = dependency('SDL2_image')
libthreads = dependency('threads')

subdir('third-party')

common = declare_dependency(
  include_directories: 'include',
  dependencies: libmsgpack,
)

subdir('libcygnet')
subdir('libswan')
subdir('core.mod')

executable('swan', 'src/main.cc',
  install: true,
  install_rpath: get_option('libdir'),
  dependencies: [
    libswan, libcygnet, common, libimgui, libsdl2_image, libgl, libglfw3, libtracy,
  ],
)

install_subdir('assets', install_dir: '')
