add_library(libswan SHARED
	src/Animation.cc
	src/Asset.cc
	src/Body.cc
	src/Chunk.cc
	src/Game.cc
	src/Item.cc
	src/Mod.cc
	src/SRF.cc
	src/Tile.cc
	src/Timer.cc
	src/World.cc
	src/WorldPlane.cc)
target_include_directories(libswan PUBLIC "include/swan")
set_target_properties(libswan PROPERTIES OUTPUT_NAME swan)
target_link_libraries(libswan ${LIBRARIES})

install(TARGETS libswan DESTINATION swan/libswan)
