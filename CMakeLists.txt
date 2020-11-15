cmake_minimum_required(VERSION 3.0)
project(swan)

find_package(SDL2 REQUIRED)
find_package(PNG)

option(USE_CLANG_TIDY "Use clang-tidy for additional checks" OFF)
if(USE_CLANG_TIDY)
	set(CMAKE_CXX_CLANG_TIDY
		clang-tidy
		"--header-filter=(libswan|core.mod|src)/.*"
		--checks=-*,bugprone-*,cert-*,performance-*,clang-analyzer-*,-cert-dcl16-c,-cert-err58-cpp,-clang-analyzer-optin.cplusplus.VirtualCall)
endif()

# Ninja runs commands with buffered stdio so the compiler won't show colors by default,
# but Ninja will strip out color codes if Ninja's stdout isn't a tty so unconditionally
# enabling color diagnostics is safe
if(CMAKE_GENERATOR STREQUAL "Ninja")
	add_compile_options(-fdiagnostics-color=always)
endif()

add_compile_options(-std=c++2a -Wall -Wextra -Wpedantic -Wno-unused-parameter)
set(libraries
	third-party imgui fmt cpptoml msgpack
	SDL2 SDL2_image dl z)

if(CMAKE_BUILD_TYPE STREQUAL Sanitize OR CMAKE_BUILD_TYPE STREQUAL "")
	message(STATUS "Build mode: Sanitize")
	add_compile_options(-g -Og -fsanitize=address -fsanitize=undefined)
	add_link_options(-fsanitize=address -fsanitize=undefined)

elseif(CMAKE_BUILD_TYPE STREQUAL Debug)
	message(STATUS "Build mode: Debug")
	add_compile_options(-g -Og)

elseif(CMAKE_BUILD_TYPE STREQUAL Tracy)
	message(STATUS "Build mode: Tracy")
	add_compile_options(-O3 -flto -DNDEBUG -g -DTRACY_ENABLE -DTRACY_ON_DEMAND)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
	list(APPEND libraries tracy)

elseif(CMAKE_BUILD_TYPE STREQUAL Release)
	message(STATUS "Build mode: Release")
	add_compile_options(-O3 -flto -DNDEBUG -g)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")

else()
	message(FATAL_ERROR "CMAKE_BUILD_TYPE must be Sanitize, Debug, DebugRelease, Tracy or Release.")
endif()

# We want to be able to use C++20 designated initializers,
# but Clang doesn't support them yet.
# Remove once Clang 9.1 or something comes out.
add_compile_options(-Wno-c99-extensions)

include_directories(${SDL2_INCLUDE_DIRS})
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;${CMAKE_INSTALL_PREFIX}/lib64;${CMAKE_INSTALL_PREFIX}/swan/libswan;${CMAKE_INSTALL_PREFIX}/swan/third-party")

add_subdirectory(third-party)
add_subdirectory(tracy-tools)
add_subdirectory(libswan)
add_subdirectory(core.mod)

add_executable(swan
	src/main.cc)
target_link_libraries(swan libswan ${libraries})

add_executable(perlin-test EXCLUDE_FROM_ALL
	src/perlin-test.cc)
target_link_libraries(perlin-test libswan PNG::PNG ${libraries})

add_executable(lighting-test EXCLUDE_FROM_ALL
	src/lighting-test.cc)
target_link_libraries(lighting-test libswan PNG::PNG ${libraries})

set(assets
	assets/icon.png
	assets/music/happy-1.wav)
foreach(a ${assets})
	configure_file("${a}" "${a}" COPYONLY)
endforeach(a)

install(TARGETS swan DESTINATION swan)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets DESTINATION swan)

add_custom_target(check DEPENDS check_libswan)
