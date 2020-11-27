add_library(libcygnet SHARED
	src/shaders/basic.cc
	src/Context.cc
	src/Program.cc
	src/Renderer.cc
	src/Window.cc)
target_include_directories(libcygnet
	PUBLIC "include"
	PRIVATE "include/cygnet")
set_target_properties(libcygnet PROPERTIES OUTPUT_NAME cygnet)
target_link_libraries(libcygnet GLESv2 ${libraries})

install(TARGETS libcygnet DESTINATION swan/libcygnet)
