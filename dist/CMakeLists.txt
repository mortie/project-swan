cmake_minimum_required(VERSION 3.0)
project(swan-dist)
include(ExternalProject)

ExternalProject_Add(compiler
	GIT_REPOSITORY https://github.com/llvm/llvm-project.git
	GIT_TAG llvmorg-9.0.0
	GIT_SHALLOW true
	GIT_PROGRESS true
	SOURCE_SUBDIR llvm
	LIST_SEPARATOR ::
	CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=Release
		-DLLVM_ENABLE_PROJECTS=clang::clang-tools-extra::libcxx::libcxxabi::libunwind::compiler-rt::lld
		-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/prefix)

set(PREFIX_DIR ${PROJECT_BINARY_DIR}/prefix)

set(toolchain
	-DCMAKE_CXX_COMPILER=${PREFIX_DIR}/bin/clang++
	-DCMAKE_C_COMPILER=${PREFIX_DIR}/bin/clang
	-DCMAKE_LINKER=${PREFIX_DIR}/bin/lld
	-DCMAKE_PREFIX_PATH=${PREFIX})

ExternalProject_Add(sfml
	GIT_REPOSITORY https://github.com/SFML/SFML.git
	GIT_TAG 2.5.1
	GIT_SHALLOW true
	GIT_PROGRESS true
	CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=Release
		-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/prefix
		${toolchain}
	DEPENDS compiler)

ExternalProject_Add(swan
	SOURCE_DIR ${PROJECT_SOURCE_DIR}/..
	CMAKE_ARGS
		-DCMAKE_INSTALL_PREFIX=${PROJECT_BINARY_DIR}/prefix
		${toolchain}
	DEPENDS compiler sfml)
