libcpptoml = declare_dependency(
  include_directories: 'cpptoml/include'
)

libfmt = declare_dependency(
  include_directories: 'fmt/include',
  link_with: library('fmt',
    'fmt/src/format.cc',
    'fmt/src/os.cc',
    include_directories: 'fmt/include',
  ),
)

libimgui = declare_dependency(
  include_directories: ['.', 'imgui'],
  link_with: static_library('imgui',
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
  ),
)

libperlinnoise = declare_dependency(
  include_directories: 'PerlinNoise',
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
  link_with: static_library('stb_truetype',
    'stb-support/stb_truetype.cc',
    include_directories: '.',
  ),
)

libstb_image = declare_dependency(
  include_directories: '.',
  link_with: static_library('stb_image',
    'stb-support/stb_image.cc',
    include_directories: '.',
  ),
)

libstb_vorbis = declare_dependency(
  include_directories: 'stb-support/include',
  link_with: static_library('stb_vorbis',
    'stb-support/stb_vorbis.cc',
  ),
)
