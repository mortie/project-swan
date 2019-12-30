add_library(libswan SHARED
	src/Animation.cc
	src/Body.cc
	src/Chunk.cc
	src/Clock.cc
	src/Game.cc
	src/Item.cc
	src/Mod.cc
	src/OS.cc
	src/Resource.cc
	src/SRF.cc
	src/Tile.cc
	src/World.cc
	src/WorldPlane.cc)
target_include_directories(libswan
	PUBLIC "include"
	PRIVATE "include/swan")
set_target_properties(libswan PROPERTIES OUTPUT_NAME swan)
target_link_libraries(libswan ${libraries})

install(TARGETS libswan DESTINATION swan/libswan)

add_executable(test_libswan EXCLUDE_FROM_ALL
	test/lib/test.cc
	test/Animation.t.cc
	test/Body.t.cc
	test/BoundingBox.t.cc
	test/Clock.t.cc
	test/common.t.cc
	test/Entity.t.cc
	test/Game.t.cc
	test/Item.t.cc
	test/Mod.t.cc
	test/OS.t.cc
	test/Resource.t.cc
	test/SRF.t.cc
	test/swan.t.cc
	test/Tile.t.cc
	test/util.t.cc
	test/log.t.cc
	test/Vector2.t.cc
	test/Win.t.cc
	test/World.t.cc
	test/WorldGen.t.cc
	test/WorldPlane.t.cc)
target_link_libraries(test_libswan libswan)
target_include_directories(test_libswan
	PRIVATE "include/swan")

add_custom_target(check_libswan
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_libswan
	DEPENDS test_libswan)
