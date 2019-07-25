cmake_minimum_required(VERSION 2.8)
project(swan)

find_package(SFML 2.5 COMPONENTS graphics system window REQUIRED)

add_compile_options(-std=c++17 -Wall -Wextra -Wpedantic -Wno-unused-parameter)
if(CMAKE_BUILD_TYPE STREQUAL Debug OR CMAKE_BUILD_TYPE STREQUAL "")
	message(STATUS "Build mode: Debug")
	add_compile_options(-g -fsanitize=address)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	message(STATUS "Build mode: Release")
	add_compile_options(-O3 -flto)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
else()
	message(FATAL_ERROR "CMAKE_BUILD_TYPE must be Debug or Release.")
endif()

include_directories("${PROJECT_SOURCE_DIR}/libswan/include")

add_subdirectory(libswan)
add_subdirectory(core.mod)

add_executable(swan src/main.cc)
target_link_libraries(swan libswan sfml-graphics sfml-system sfml-window dl)
