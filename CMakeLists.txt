cmake_minimum_required(VERSION 2.8)
project(swan)

find_package(SFML 2.5 COMPONENTS graphics system window REQUIRED)

add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
include_directories("${PROJECT_SOURCE_DIR}/libswan/include")

add_subdirectory(libswan)
add_subdirectory(core.mod)

add_executable(swan src/main.cc)
target_link_libraries(swan libswan sfml-graphics sfml-system sfml-window dl)
