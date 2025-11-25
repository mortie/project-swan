swan_proto = custom_target(
  'swan_proto',
  input: 'proto/swan.capnp',
  output: ['swan.capnp.h', 'swan.capnp.c++'],
  command: [
    capnp_prog, 'compile', '-oc++:@OUTDIR@',
    '--src-prefix=@CURRENT_SOURCE_DIR@/proto',
    '@INPUT@',
  ],
  install: true,
  install_dir: ['include/proto', false],
)
install_data(
  'proto/swan.capnp',
  install_dir: 'include/proto',
)

libswan_cflags = []
if has_ffmpeg
  libswan_cflags += '-DSWAN_FFMPEG_AVAILABLE'
endif

libswan = declare_dependency(
  include_directories: ['include', '.'],
  dependencies: [
    common, libcygnet, libglfw3, libcapnp,
  ] + maybe_libtracy,
  link_with: shared_library('swan',
    'src/inputmaps/axes.cc',
    'src/inputmaps/buttons.cc',
    'src/inputmaps/keys.cc',
    'src/inputmaps/mouse.cc',
    'src/systems/EntitySystem.cc',
    'src/systems/FluidSystem.cc',
    'src/systems/LightSystem.cc',
    'src/systems/TileSystem.cc',
    'src/traits/InventoryTrait.cc',
    'src/traits/PhysicsBodyTrait.cc',
    'src/Animation.cc',
    'src/assets.cc',
    'src/Chunk.cc',
    'src/Clock.cc',
    'src/uiutil.cc',
    'src/EntityCollection.cc',
    'src/FrameRecorder.cc',
    'src/Game.cc',
    'src/InputHandler.cc',
    'src/ItemStack.cc',
    'src/LightServer.cc',
    'src/Mod.cc',
    'src/OS.cc',
    'src/SoundPlayer.cc',
    'src/Tile.cc',
    'src/World.cc',
    'src/WorldPlane.cc',
    swan_proto,
    dependencies: [
      common, libcygnet, libthreads, libstb_image,
      libstb_vorbis, libcpptoml, libdl, libz, libgl, libglfw3,
      libimgui, libportaudio, libcapnp, libffmpeg, libdate,
    ] + maybe_libtracy,
    cpp_args: libswan_cflags,
    install: true,
    install_rpath: origin_rpath,
    include_directories: 'include/swan',
  ),
)
install_subdir(
  'include/swan',
  install_dir: 'include',
)

executable(
  'libswan_test',
  'test/lib/test.cc',
  'test/ItemStack.t.cc',
  dependencies: libswan,
  include_directories: 'include/swan',
)
