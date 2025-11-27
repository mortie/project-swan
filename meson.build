project(
  'swan', 'cpp',
  default_options: [
    'cpp_std=c++20',
    'libdir=lib',
    'bindir=bin',
    'warning_level=3',
  ],
)

cmake = import('cmake')

shaderpp_prog = find_program('./scripts/shaderpp.py')

if host_machine.system() == 'darwin'
  origin_rpath = '@loader_path'
else
  origin_rpath = '$ORIGIN'
endif
bin_rpath = origin_rpath + '/../' + get_option('libdir')

add_project_arguments('-Wno-unused-parameter', language: 'cpp')
if meson.get_compiler('cpp').get_id() == 'clang'
  add_global_arguments('-Werror=reorder-init-list', language : 'cpp')
endif

maybe_libtracy = []
if get_option('trace')
  tracy_proj = subproject('tracy')
  maybe_libtracy = [tracy_proj.get_variable('tracy_dep')]
  add_project_arguments('-DTRACY_ENABLE', language: 'cpp')
endif

glfw_opts = cmake.subproject_options()
glfw_opts.add_cmake_defines({
  'GLFW_LIBRARY_TYPE': 'SHARED',
  'GLFW_BUILD_EXAMPLES': false,
  'GLFW_BUILD_TESTS': false,
  'GLFW_BUILD_DOCS': false,
})
glfw_opts.set_override_option('warning_level', '1')
glfw_sub = cmake.subproject('glfw', options: glfw_opts)
libglfw3 = glfw_sub.dependency('glfw')

portaudio_opts = cmake.subproject_options()
portaudio_opts.add_cmake_defines({
  'CMAKE_POLICY_VERSION_MINIMUM': '3.5',
  'PA_BUILD_STATIC': false,
})
portaudio_opts.set_override_option('warning_level', '0')
portaudio_sub = cmake.subproject('portaudio', options: portaudio_opts)
libportaudio = portaudio_sub.dependency('portaudio')

libffmpeg = [
  dependency('libavcodec', required: false),
  dependency('libavutil', required: false),
  dependency('libswscale', required: false),
  dependency('libavformat', required: false),
]

has_ffmpeg = true
foreach lib: libffmpeg
  if not lib.found()
    has_ffmpeg = false
    break
  endif
endforeach

libcpptoml = subproject('cpptoml-ng').get_variable('cpptoml_dep')
libdl = meson.get_compiler('cpp').find_library('dl')
libz = meson.get_compiler('cpp').find_library('z')

libgl = dependency('OpenGL')
libdw = dependency('libdw', required: false)

libthreads = dependency('threads')

# The game engine doesn't use scisa,
# but we want it available to mods
scisa_sub = subproject('scisa')
libscisasm = scisa_sub.get_variable('libscisasm')
libscisavm = scisa_sub.get_variable('libscisavm')

subdir('third-party')

common = declare_dependency(
  include_directories: 'include',
)

subdir('libcygnet')
subdir('libswan')
subdir('launcher')
install_subdir(
  'include/swan',
  install_dir: 'include',
)

executable(
  'swan',
  'src/build.cc',
  'src/swan.cc',
  swan_proto,
  install: true,
  install_rpath: bin_rpath,
  dependencies: [
    libswan, libcygnet, common, libimgui, libgl, libglfw3, libbackwardcpp,
    libcpptoml, libsha1, libscisasm, libscisavm,
  ] + maybe_libtracy,
  override_options: ['b_asneeded=false'],
)

executable(
  'swan-build',
  'src/build.cc',
  'src/swan-build.cc',
  install: true,
  install_rpath: bin_rpath,
  dependencies: [
    libcpptoml,
    libsha1,
  ],
)

install_subdir('assets', install_dir: '')
install_subdir(
  'core.mod',
  install_dir: '',
  exclude_directories: [
    '.swanbuild',
    '.cache',
    'assets-src',
  ],
  exclude_files: [
    'compile_commands.json',
  ],
)
