include_directories("${CMAKE_CURRENT_LIST_DIR}/include/swan")
add_library(libswan SHARED
	src/Body.cc
	src/Chunk.cc
	src/Game.cc
	src/Mod.cc
	src/Player.cc
	src/World.cc
	src/WorldPlane.cc)
set_target_properties(libswan PROPERTIES OUTPUT_NAME swan)
