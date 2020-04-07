add_library(core.mod SHARED
	src/main.cc
	src/DefaultWorldGen.cc
	src/entities/PlayerEntity.cc
	src/entities/ItemStackEntity.cc)
set_target_properties(core.mod PROPERTIES OUTPUT_NAME mod PREFIX "")
target_link_libraries(core.mod libswan ${libraries})

set(assets
	assets/entity/player-still.png
	assets/entity/player-still.toml
	assets/entity/player-running.png
	assets/entity/player-running.toml
	assets/tile/grass.png
	assets/tile/air.png
	assets/tile/stone.png
	assets/tile/dirt.png
	assets/tile/leaves.png
	assets/tile/tree-trunk.png
	assets/misc/background-cave.png)
foreach(a ${assets})
	configure_file("${a}" "${a}" COPYONLY)
endforeach(a)

install(TARGETS core.mod DESTINATION swan/core.mod)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets DESTINATION swan/core.mod)
