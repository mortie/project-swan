cmake_minimum_required(VERSION 3.0)
project(swan-dist)
include(ExternalProject)

set(prefix ${PROJECT_BINARY_DIR}/prefix)

set(common
	-DCMAKE_INSTALL_PREFIX=${prefix}
	-DCMAKE_PREFIX_PATH=${prefix})

ExternalProject_Add(sfml
	GIT_REPOSITORY https://github.com/SFML/SFML.git
	GIT_TAG 2.5.1
	GIT_SHALLOW true
	GIT_PROGRESS true
	CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=Release
		${common})

ExternalProject_Add(imgui-sfml
	GIT_REPOSITORY https://github.com/eliasdaler/imgui-sfml.git
	GIT_TAG v2.1
	GIT_SHALLOW true
	GIT_PROGRESS true
	CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=Release
		-DIMGUI_DIR=${PROJECT_SOURCE_DIR}/../third_party/imgui
		-DBUILD_SHARED_LIBS=true
		${common}
	DEPENDS sfml)

ExternalProject_Add(swan
	SOURCE_DIR ${PROJECT_SOURCE_DIR}/..
	CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
		${common}
	DEPENDS sfml imgui-sfml)
