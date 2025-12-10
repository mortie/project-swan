shader_gen = generator(
  shaderpp_prog,
  output: ['@PLAINNAME@.h', '@PLAINNAME@.cc'],
  arguments: ['@INPUT@', '@OUTPUT0@', '@OUTPUT1@'],
)

libcygnet = declare_dependency(
  include_directories: 'include',
  link_with: shared_library('cygnet',
    'src/gl.cc',
    'src/GlWrappers.cc',
    'src/Gui.cc',
    'src/Renderer.cc',
    'src/ResourceManager.cc',
    'src/TextCache.cc',
    'src/TileAtlas.cc',
    'src/util.cc',
    shader_gen.process(
      'src/glsl/Blend.glsl',
      'src/glsl/Chunk.glsl',
      'src/glsl/ChunkFluid.glsl',
      'src/glsl/ChunkShadow.glsl',
      'src/glsl/Mask.glsl',
      'src/glsl/Particle.glsl',
      'src/glsl/Rect.glsl',
      'src/glsl/Sprite.glsl',
      'src/glsl/Text.glsl',
      'src/glsl/Tile.glsl',
    ),
    dependencies: [
      common, libthreads, libgl, libglfw3, libstb_truetype,
    ] + maybe_libtracy,
    install: true,
    install_rpath: origin_rpath,
    include_directories: ['include', 'include/cygnet', 'src'],
  ),
)
install_subdir(
  'include/cygnet',
  install_dir: 'include',
)
