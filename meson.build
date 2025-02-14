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
capnp_prog = find_program('capnp')

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

libglfw3 = dependency('glfw3')

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

portaudio_options = cmake.subproject_options()
portaudio_options.append_compile_args('c', '-w') # Disable warnings
portaudio_options.add_cmake_defines({
  'PA_BUILD_SHARED': true,
  'PA_BUILD_STATIC': false,
})
portaudio_sub = cmake.subproject('portaudio', options: portaudio_options)
libportaudio = portaudio_sub.dependency('portaudio')

libcapnp = dependency('capnp')

libdl = meson.get_compiler('cpp').find_library('dl')
libz = meson.get_compiler('cpp').find_library('z')

libgl = dependency('OpenGL')
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
