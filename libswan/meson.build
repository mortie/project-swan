libswan = declare_dependency(
  include_directories: 'include',
  dependencies: [
    common, libcygnet, libglfw3, libmsgstream,
  ] + maybe_libtracy,
  link_with: library('swan',
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
    dependencies: [
      common, libcygnet, libthreads, libstb_image,
      libstb_vorbis, libcpptoml, libdl, libz, libglfw3,
      libimgui, libportaudio, libmsgstream,
    ] + maybe_libtracy,
    install: true,
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
