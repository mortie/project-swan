swan_proto = custom_target(
  'swan_proto',
  input: 'proto/swan.capnp',
  output: ['swan.capnp.h', 'swan.capnp.c++'],
  command: [
    capnp_prog, 'compile', '-oc++:@OUTDIR@',
    '--src-prefix=@CURRENT_SOURCE_DIR@/proto',
    '@INPUT@',
  ],
)

libswan = declare_dependency(
  include_directories: ['include', '.'],
  dependencies: [
    common, libcygnet, libglfw3, libsbon, libcapnp,
  ] + maybe_libtracy,
  link_with: library('swan',
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
    'src/drawutil.cc',
    'src/EntityCollection.cc',
    'src/Game.cc',
    'src/ItemStack.cc',
    'src/LightServer.cc',
    'src/OS.cc',
    'src/SoundPlayer.cc',
    'src/World.cc',
    'src/WorldPlane.cc',
    swan_proto,
    dependencies: [
      common, libcygnet, libthreads, libstb_image,
      libstb_vorbis, libcpptoml, libdl, libz, libglfw3,
      libimgui, libportaudio, libsbon, libcapnp,
    ] + maybe_libtracy,
    install: true,
    install_rpath: get_option('libdir'),
    include_directories: 'include/swan',
  ),
)

executable(
  'libswan_test',
  'test/lib/test.cc',
  'test/ItemStack.t.cc',
  dependencies: libswan,
  include_directories: 'include/swan',
)
