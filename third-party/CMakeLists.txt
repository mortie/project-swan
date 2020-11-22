add_library(third-party INTERFACE)
target_include_directories(third-party SYSTEM INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR})

add_library(cpptoml INTERFACE)
target_include_directories(cpptoml SYSTEM INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/cpptoml/include)

add_library(fmt SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/fmt/src/format.cc
	${CMAKE_CURRENT_SOURCE_DIR}/fmt/src/os.cc)
target_include_directories(fmt SYSTEM PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/fmt/include)
set_target_properties(fmt PROPERTIES CXX_CLANG_TIDY "")
install(TARGETS fmt DESTINATION swan/third-party)

add_library(imgui SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui_sdl/imgui_sdl.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui-plot/src/imgui_plot.cpp)
target_include_directories(imgui SYSTEM PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/imgui
	${CMAKE_CURRENT_SOURCE_DIR}/imgui-plot/include)
target_link_libraries(imgui SDL2::SDL2)
set_target_properties(imgui PROPERTIES CXX_CLANG_TIDY "")
target_compile_options(imgui PRIVATE -Wno-sign-compare)
install(TARGETS imgui DESTINATION swan/third-party)

add_library(msgpack INTERFACE)
target_include_directories(msgpack SYSTEM INTERFACE
	${CMAKE_CURRENT_SOURCE_DIR}/msgpack-c/include)

add_library(tracy SHARED
	${CMAKE_CURRENT_SOURCE_DIR}/tracy/TracyClient.cpp)
set_target_properties(tracy PROPERTIES CXX_CLANG_TIDY "")
target_compile_options(tracy PRIVATE
	-Wno-unused-function -Wno-unused-result -Wno-sign-compare -Wno-pedantic
	-Wno-enum-compare -Wno-unused-variable)
install(TARGETS tracy DESTINATION swan/third-party)
