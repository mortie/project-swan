cmake_minimum_required(VERSION 3.0)
project(swan)

option(USE_CLANG_TIDY "Use clang-tidy for additional checks" ON)
if(USE_CLANG_TIDY)
	set(CMAKE_CXX_CLANG_TIDY
		clang-tidy
		--header-filter=.*
		--checks=-*,bugprone-*,cert-*,performance-*,clang-analyzer-*,-cert-dcl16-c,-cert-err58-cpp,-clang-analyzer-optin.cplusplus.VirtualCall)
endif()

add_compile_options(-std=c++2a -Wall -Wextra -Wpedantic -Wno-unused-parameter)
if(CMAKE_BUILD_TYPE STREQUAL Sanitize OR CMAKE_BUILD_TYPE STREQUAL "")
	message(STATUS "Build mode: Sanitize")
	add_compile_options(-g -Og -fsanitize=address -fsanitize=undefined)
	add_link_options(-fsanitize=address -fsanitize=undefined)
elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
	message(STATUS "Build mode: Debug")
	add_compile_options(-g -O0)
elseif(CMAKE_BUILD_TYPE STREQUAL Profile)
	message(STATUS "Build mode: Profile")
	add_compile_options(-g -Og -pg)
	add_link_options(-pg)
elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	message(STATUS "Build mode: Release")
	add_compile_options(-O3 -flto)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
else()
	message(FATAL_ERROR "CMAKE_BUILD_TYPE must be Debug or Release.")
endif()

add_subdirectory(third_party)

set(libraries imgui SDL2 SDL2_image dl z)

# We want to be able to use C++20 designated initializers,
# but Clang doesn't support them yet.
# Remove once Clang 9.1 or something comes out.
add_compile_options(-Wno-c99-extensions)

include_directories(
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

add_custom_target(check DEPENDS check_libswan)
