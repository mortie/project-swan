core_mod_proto = custom_target(
  'core_mod_proto',
  input: 'proto/core_mod.capnp',
  output: ['core_mod.capnp.h', 'core_mod.capnp.c++'],
  command: [
    capnp_prog, 'compile', '-oc++:@OUTDIR@',
    '--src-prefix=@CURRENT_SOURCE_DIR@/proto',
    '-I@SOURCE_ROOT@/libswan/proto',
    '@INPUT@',
  ],
)

core_mod = shared_library('mod',
  'src/main.cc',
  'src/DefaultWorldGen.cc',
  'src/entities/DynamiteEntity.cc',
  'src/entities/FallingTileEntity.cc',
  'src/entities/PlayerEntity.cc',
  'src/entities/ItemStackEntity.cc',
  'src/entities/SpiderEntity.cc',
  'src/tileentities/ItemPipeTileEntity.cc',
  'src/tileentities/ItemFanTileEntity.cc',
  'src/world/item-fan.cc',
  'src/world/ladder.cc',
  'src/world/outcrop.cc',
  'src/world/pipe.cc',
  'src/world/torch.cc',
  'src/world/tree.cc',
  'src/worldgen/ShrubberyDef.cc',
  'src/worldgen/TallGrassDef.cc',
  'src/worldgen/TreeDef.cc',
  core_mod_proto,
  name_prefix: '',
  name_suffix: 'so',
  install: true,
  install_dir: 'core.mod',
  include_directories: 'src',
  dependencies: [libswan, libperlinnoise, libimgui]
)

install_subdir('assets', install_dir: 'core.mod')
