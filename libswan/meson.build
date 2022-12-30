libswan_deps = [
  common, libcygnet, libthreads, libsdl2_image,
  libcpptoml, libdl, libz, libglfw3, libimgui, libtracy
]

libswan = declare_dependency(
  include_directories: 'include',
  dependencies: libswan_deps,
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
    dependencies: libswan_deps,
    install: true,
    include_directories: 'include/swan',
  ),
)
