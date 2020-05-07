add_library(cygnet SHARED
	src/GlProgram.cc
	src/Window.cc)
target_link_libraries(cygnet PUBLIC SDL2 GLESv2)
target_include_directories(cygnet
	PUBLIC include
	PRIVATE include/cygnet)

add_executable(cygnet-hello-triangle
	samples/hello-triangle/hello-triangle.cc)
target_link_libraries(cygnet-hello-triangle PUBLIC cygnet)
