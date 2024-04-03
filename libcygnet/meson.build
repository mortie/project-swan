shader_gen = generator(
  shaderpp_prog,
  output: ['@PLAINNAME@.h', '@PLAINNAME@.cc'],
  arguments: ['@INPUT@', '@OUTPUT0@', '@OUTPUT1@'],
)

libcygnet = declare_dependency(
  include_directories: 'include',
  link_with: library('cygnet',
    'src/gl.cc',
    'src/GlWrappers.cc',
    'src/Renderer.cc',
    'src/ResourceManager.cc',
    'src/TextCache.cc',
    'src/TileAtlas.cc',
    'src/util.cc',
    shader_gen.process(
      'src/glsl/Blend.glsl',
      'src/glsl/Chunk.glsl',
      'src/glsl/ChunkShadow.glsl',
      'src/glsl/Rect.glsl',
      'src/glsl/Sprite.glsl',
      'src/glsl/Tile.glsl',
    ),
    dependencies: [
      common, libthreads, libgl, libglfw3, libstb_truetype,
    ] + maybe_libtracy,
    install: true,
    install_rpath: get_option('libdir'),
    include_directories: 'include/cygnet',
  ),
)
