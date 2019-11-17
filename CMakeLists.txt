cmake_minimum_required(VERSION 3.0)
project(swan)

find_package(SFML 2.5 COMPONENTS graphics system window REQUIRED)
find_package(ImGui-SFML REQUIRED)

set(CMAKE_CXX_CLANG_TIDY
	clang-tidy
	--header-filter=.*
	--checks=-*,bugprone-*,cert-*,performance-*,clang-analyzer-*,-cert-dcl16-c,-cert-err58-cpp,-clang-analyzer-optin.cplusplus.VirtualCall)

add_compile_options(-std=c++2a -Wall -Wextra -Wpedantic -Wno-unused-parameter)
if(CMAKE_BUILD_TYPE STREQUAL Sanitize OR CMAKE_BUILD_TYPE STREQUAL "")
	message(STATUS "Build mode: Sanitize")
	add_compile_options(-g -fsanitize=address)
	add_link_options(-fsanitize=address)
elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
	message(STATUS "Build mode: Debug")
	add_compile_options(-g)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	message(STATUS "Build mode: Release")
	add_compile_options(-O3 -flto)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
else()
	message(FATAL_ERROR "CMAKE_BUILD_TYPE must be Debug or Release.")
endif()

add_subdirectory(third_party)

set(libraries
	sfml-graphics sfml-system sfml-window sfml-audio ImGui-SFML::ImGui-SFML
	imgui SDL2 SDL2_image dl z)

# We want to be able to use C++20 designated initializers,
# but Clang doesn't support them yet.
# Remove once Clang 9.1 or something comes out.
add_compile_options(-Wno-c99-extensions)

include_directories(
	${PROJECT_SOURCE_DIR}/libswan/include
	${PROJECT_SOURCE_DIR}/third_party)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/lib64;${CMAKE_INSTALL_PREFIX}/swan/libswan;${CMAKE_INSTALL_PREFIX}/swan/third_party")

add_subdirectory(libswan)
add_subdirectory(core.mod)

add_executable(swan
	src/main.cc)
target_link_libraries(swan libswan ${libraries})

set(assets
	assets/icon.png
	assets/music/happy-1.wav)
foreach(a ${assets})
	configure_file("${a}" "${a}" COPYONLY)
endforeach(a)

install(TARGETS swan DESTINATION swan)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets DESTINATION swan)
