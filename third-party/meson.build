libfmt = declare_dependency(
  include_directories: 'fmt/include',
  link_with: shared_library('fmt',
    'fmt/src/format.cc',
    'fmt/src/os.cc',
    include_directories: 'fmt/include',
    install: true,
    install_rpath: get_option('libdir'),
  ),
)

libimgui = declare_dependency(
  include_directories: ['.', 'imgui'],
  link_with: shared_library('imgui',
    'imgui/backends/imgui_impl_opengl3.cpp',
    'imgui/backends/imgui_impl_glfw.cpp',
    'imgui/imgui_demo.cpp',
    'imgui/imgui_draw.cpp',
    'imgui/imgui_tables.cpp',
    'imgui/imgui_widgets.cpp',
    'imgui/imgui.cpp',
    include_directories: 'imgui',
    cpp_args: ['-Wno-sign-compare', '-Wno-deprecated-enum-enum-conversion'],
    dependencies: [libglfw3],
    install: true,
    install_rpath: get_option('libdir'),
  ),
)
install_headers(
  'imgui/imgui.h',
  'imgui/imconfig.h',
  install_dir: 'include/imgui',
)

libperlinnoise = declare_dependency(
  include_directories: 'PerlinNoise',
)
install_headers(
  'PerlinNoise/PerlinNoise.hpp',
  install_dir: 'include',
)

if libdw.found()
  libbackwardcpp = declare_dependency(
    include_directories: 'backward-cpp',
    compile_args: ['-DBACKWARD_HAS_DW=1'],
    dependencies: [libdw],
  )
else
  libbackwardcpp = declare_dependency(
    include_directories: 'backward-cpp',
  )
endif

libstb_truetype = declare_dependency(
  include_directories: '.',
  link_with: shared_library('stb_truetype',
    'stb-support/stb_truetype.cc',
    include_directories: '.',
    install: true,
    install_rpath: get_option('libdir'),
  ),
)

libstb_image = declare_dependency(
  include_directories: '.',
  link_with: shared_library('stb_image',
    'stb-support/stb_image.cc',
    include_directories: '.',
    install: true,
    install_rpath: get_option('libdir'),
  ),
)

libstb_vorbis = declare_dependency(
  include_directories: 'stb-support/include',
  link_with: shared_library('stb_vorbis',
    'stb-support/stb_vorbis.cc',
    install: true,
    install_rpath: get_option('libdir'),
  ),
)

libsha1 = declare_dependency(
  include_directories: '.',
  link_with: shared_library('libsha1',
    'sha1/sha1.cpp',
    install: true,
    install_rpath: get_option('libdir'),
  ),
)
