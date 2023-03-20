libswan = declare_dependency(
  include_directories: 'include',
  dependencies: [
    common, libcygnet, libglfw3,
  ] + maybe_libtracy,
  link_with: library('swan',
    'src/traits/InventoryTrait.cc',
    'src/traits/PhysicsTrait.cc',
    'src/Animation.cc',
    'src/assets.cc',
    'src/Chunk.cc',
    'src/Clock.cc',
    'src/drawutil.cc',
    'src/Game.cc',
    'src/ItemStack.cc',
    'src/LightServer.cc',
    'src/OS.cc',
    'src/World.cc',
    'src/WorldPlane.cc',
    dependencies: [
      common, libcygnet, libthreads, libsdl2_image,
      libcpptoml, libdl, libz, libglfw3, libimgui,
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
