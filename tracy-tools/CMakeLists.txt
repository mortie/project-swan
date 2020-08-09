find_package(Freetype)
find_package(glfw3)
find_package(GTK2 REQUIRED COMPONENTS gtk)

add_library(tracy-libs-unix OBJECT EXCLUDE_FROM_ALL
	${PROJECT_SOURCE_DIR}/third-party/tracy/nfd/nfd_gtk.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/libbacktrace/posix.cpp)
target_include_directories(tracy-libs-unix PRIVATE ${GTK2_INCLUDE_DIRS})
target_link_libraries(tracy-libs-unix ${GTK2_LIBRARIES})
target_compile_options(tracy-libs-unix
	PUBLIC -UTRACY_ENABLE
	PRIVATE -Wno-deprecated-declarations)

add_library(tracy-libs STATIC EXCLUDE_FROM_ALL
	${PROJECT_SOURCE_DIR}/third-party/tracy/profiler/libs/gl3w/GL/gl3w.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/common/TracySocket.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/common/TracySystem.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/common/tracy_lz4.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/common/tracy_lz4hc.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/client/tracy_rpmalloc.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/client/TracyCallstack.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/client/TracyDxt1.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/client/TracyProfiler.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/client/TracySysTime.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/client/TracySysTrace.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/imguicolortextedit/TextEditor.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/imgui/imgui.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/imgui/imgui_demo.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/imgui/imgui_draw.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/imgui/imgui_widgets.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/nfd/nfd_common.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/libbacktrace/alloc.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/libbacktrace/dwarf.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/libbacktrace/elf.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/libbacktrace/fileline.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/libbacktrace/mmapio.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/libbacktrace/sort.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/libbacktrace/state.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyBadVersion.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyMemory.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyMmap.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyPrint.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyStorage.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyTaskDispatch.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyTexture.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyThreadCompress.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyUserData.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyView.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/server/TracyWorker.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/debug.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/entropy_common.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/error_private.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/fse_compress.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/fse_decompress.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/hist.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/huf_compress.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/huf_decompress.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/pool.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/threading.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/xxhash.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstdmt_compress.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_common.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_compress.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_compress_literals.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_compress_sequences.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_compress_superblock.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_ddict.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_decompress.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_decompress_block.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_double_fast.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_fast.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_lazy.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_ldm.c
	${PROJECT_SOURCE_DIR}/third-party/tracy/zstd/zstd_opt.c)
set_target_properties(tracy-libs PROPERTIES CXX_CLANG_TIDY "")
target_include_directories(tracy-libs PUBLIC
	${PROJECT_SOURCE_DIR}/third-party/tracy/profiler/libs/gl3w
	${PROJECT_SOURCE_DIR}/third-party/tracy/imgui)
target_link_libraries(tracy-libs tracy-libs-unix
	pthread dl Freetype::Freetype glfw GL)
target_compile_options(tracy-libs PUBLIC
	-w -UTRACY_ENABLE
	-DTRACY_FILESELECTOR -DTRACY_EXTENDED_FONT -DTRACY_ROOT_WINDOW
	-DIMGUI_IMPL_OPENGL_LOADER_GL3W)

add_executable(tracy-profiler EXCLUDE_FROM_ALL
	${PROJECT_SOURCE_DIR}/third-party/tracy/profiler/src/imgui_freetype.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/profiler/src/imgui_impl_glfw.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/profiler/src/imgui_impl_opengl3.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/profiler/src/main.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/profiler/src/ResolvService.cpp
	${PROJECT_SOURCE_DIR}/third-party/tracy/profiler/src/winmainArchDiscovery.cpp)
set_target_properties(tracy-libs PROPERTIES CXX_CLANG_TIDY "")
target_link_libraries(tracy-profiler tracy-libs tbb)
