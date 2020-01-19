add_library(imgui SHARED
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui_sdl/imgui_sdl.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui-plot/src/imgui_plot.cpp)
set_target_properties(imgui PROPERTIES CXX_CLANG_TIDY "")
target_include_directories(imgui PUBLIC
	${PROJECT_SOURCE_DIR}/third_party/imgui
	${PROJECT_SOURCE_DIR}/third_party/imgui-plot/include
	${SDL2_INCLUDE_DIRS})

install(TARGETS imgui DESTINATION swan/third_party)
