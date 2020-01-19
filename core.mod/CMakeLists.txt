add_library(core.mod SHARED
	src/main.cc
	src/WGDefault.cc
	src/entities/EntPlayer.cc
	src/entities/EntItemStack.cc)
set_target_properties(core.mod PROPERTIES OUTPUT_NAME mod PREFIX "")
target_link_libraries(core.mod libswan ${libraries})

set(assets
	assets/entities/player-still.png
	assets/entities/player-running.png
	assets/tiles/grass.png
	assets/tiles/air.png
	assets/tiles/stone.png
	assets/tiles/dirt.png
	assets/tiles/tree-trunk.png)
foreach(a ${assets})
	configure_file("${a}" "${a}" COPYONLY)
endforeach(a)

install(TARGETS core.mod DESTINATION swan/core.mod)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets DESTINATION swan/core.mod)
