project(
  'swan', 'cpp',
  default_options: ['cpp_std=c++2a'],
)

cmake = import('cmake')

shaderpp_prog = find_program('./scripts/shaderpp.py')

maybe_libtracy = []
if get_option('trace')
  tracy_proj = subproject('tracy')
  maybe_libtracy = [tracy_proj.get_variable('tracy_dep')]
  add_project_arguments('-DTRACY_ENABLE', language: 'cpp')
endif

glfw_options = cmake.subproject_options()
glfw_options.add_cmake_defines({
  'BUILD_SHARED_LIBS': true,
  'GLFW_BUILD_EXAMPLES': false,
  'GLFW_BUILD_TESTS': false,
  'GLFW_BUILD_DOCS': false,
})
glfw_sub = cmake.subproject('glfw', options: glfw_options)
libglfw3 = glfw_sub.dependency('glfw')

libdl = meson.get_compiler('cpp').find_library('dl')
libz = meson.get_compiler('cpp').find_library('z')

libgl = dependency('OpenGL')
libportaudio = dependency('portaudio-2.0')
libdw = dependency('libdw', required: false)

libthreads = dependency('threads')

subdir('third-party')

common = declare_dependency(
  include_directories: 'include',
)

subdir('libcygnet')
subdir('libswan')
subdir('core.mod')

executable('swan', 'src/main.cc',
  install: true,
  install_rpath: get_option('libdir'),
  dependencies: [
    libswan, libcygnet, common, libimgui, libgl, libglfw3, libbackwardcpp,
  ] + maybe_libtracy,
)

install_subdir('assets', install_dir: '')
