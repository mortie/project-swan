add_library(imgui SHARED
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_demo.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/third_party/imgui/imgui.cpp)
set_target_properties(imgui PROPERTIES CXX_CLANG_TIDY "")

install(TARGETS imgui DESTINATION swan/third_party)
