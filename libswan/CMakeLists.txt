add_library(libswan SHARED
	src/Body.cc
	src/Chunk.cc
	src/Game.cc
	src/Mod.cc
	src/Player.cc
	src/Tile.cc
	src/World.cc
	src/WorldPlane.cc)
target_include_directories(libswan PUBLIC "include/swan")
set_target_properties(libswan PROPERTIES OUTPUT_NAME swan)
