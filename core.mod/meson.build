core_mod = shared_library('mod',
	'src/main.cc',
	'src/DefaultWorldGen.cc',
	'src/entities/PlayerEntity.cc',
	'src/entities/ItemStackEntity.cc',
	name_prefix: '',
	install: true,
	install_dir: 'core.mod',
	dependencies: [libswan, libperlinnoise])

install_subdir('assets', install_dir: 'core.mod')
