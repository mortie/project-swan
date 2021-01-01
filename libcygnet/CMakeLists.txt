add_library(libcygnet SHARED
	src/Context.cc
	src/GlWrappers.cc
	src/Renderer.cc
	src/ResourceManager.cc
	src/shaders.cc
	src/TileAtlas.cc
	src/util.cc
	src/Window.cc)
target_include_directories(libcygnet
	PUBLIC "include"
	PRIVATE "include/cygnet")
target_link_libraries(libcygnet swan-common GLESv2 ${libraries})

install(TARGETS libcygnet DESTINATION swan/libcygnet)
