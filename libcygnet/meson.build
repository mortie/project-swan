cygnet_deps = [common, libthreads, libgl, libglfw3, libtracy, libstb_truetype]

libcygnet = declare_dependency(
  include_directories: 'include',
  link_with: library('cygnet',
    'src/gl.cc',
    'src/GlWrappers.cc',
    'src/Renderer.cc',
    'src/ResourceManager.cc',
    'src/shaders.cc',
    'src/TextCache.cc',
    'src/TileAtlas.cc',
    'src/util.cc',
    dependencies: cygnet_deps,
    install: true,
    include_directories: 'include/cygnet',
  ),
)
