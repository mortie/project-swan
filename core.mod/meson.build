core_mod = shared_library('mod',
  'src/main.cc',
  'src/DefaultWorldGen.cc',
  'src/entities/DynamiteEntity.cc',
  'src/entities/FallingTileEntity.cc',
  'src/entities/PlayerEntity.cc',
  'src/entities/ItemStackEntity.cc',
  'src/entities/SpiderEntity.cc',
  'src/world/ladder.cc',
  'src/world/pipe.cc',
  'src/world/torch.cc',
  'src/world/tree.cc',
  'src/worldgen/TallGrassDef.cc',
  'src/worldgen/TreeDef.cc',
  name_prefix: '',
  name_suffix: 'so',
  install: true,
  install_dir: 'core.mod',
  include_directories: 'src',
  dependencies: [libswan, libperlinnoise, libimgui]
)

install_subdir('assets', install_dir: 'core.mod')
