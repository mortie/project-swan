add_library(imgui SHARED
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui_sdl/imgui_sdl.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui-plot/src/imgui_plot.cpp)
target_include_directories(imgui PUBLIC
	${PROJECT_SOURCE_DIR}/third_party/imgui
	${PROJECT_SOURCE_DIR}/third_party/imgui-plot/include
	${SDL2_INCLUDE_DIRS})
set_target_properties(imgui PROPERTIES CXX_CLANG_TIDY "")
install(TARGETS imgui DESTINATION swan/third_party)

add_library(fmt SHARED
	${PROJECT_SOURCE_DIR}/third_party/fmt/src/format.cc
	${PROJECT_SOURCE_DIR}/third_party/fmt/src/os.cc)
target_include_directories(fmt PUBLIC
	${PROJECT_SOURCE_DIR}/third_party/fmt/include)
install(TARGETS fmt DESTINATION swan/third_party)

add_library(cpptoml INTERFACE)
target_include_directories(cpptoml INTERFACE
	${PROJECT_SOURCE_DIR}/third_party/cpptoml/include)

add_library(msgpack INTERFACE)
target_include_directories(msgpack INTERFACE
	${PROJECT_SOURCE_DIR}/third_party/msgpack-c/include)

