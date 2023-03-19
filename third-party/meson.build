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

libmsgpack = declare_dependency(
  include_directories: 'msgpack-c/include'
)

libperlinnoise = declare_dependency(
  include_directories: 'PerlinNoise'
)

libstb_truetype = declare_dependency(
  include_directories: '.',
  link_with: static_library('stb_truetype',
    'stb_impls/stb_truetype.cc',
    include_directories: '.',
  ),
)
