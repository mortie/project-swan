add_library(libswan SHARED
	src/traits/BodyTrait.cc
	src/Animation.cc
	src/Chunk.cc
	src/Clock.cc
	src/Game.cc
	src/gfxutil.cc
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
	test/util.t.cc)
target_link_libraries(test_libswan libswan)
target_include_directories(test_libswan
	PRIVATE "include/swan")

add_custom_target(check_libswan
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_libswan
	DEPENDS test_libswan)
