#!/bin/sh

set -ex

cd "$(dirname "$0")"
TOP="$PWD"
PFX="$TOP/pfx"

os="$(uname -s)"
cpu="$(uname -m)"

echo
echo "Preparing prefix..."
rm -rf "$PFX"
mkdir -p "$PFX"

get_source() {
	if [ -e "$1/.checkout.stamp" ]; then
		return
	fi

	rm -rf "$1"
	git clone "$2" "$1"
	(cd "$1" && git checkout "$3" && git submodule update --init --recursive)
	touch "$1/.checkout.stamp"
}

echo
echo "Setting up sources..."
mkdir -p sources
get_source sources/cmake https://github.com/Kitware/CMake.git \
	a0c7f1d29c77fd5c862b087f9d2442c84798a4b6
get_source sources/ffmpeg https://git.ffmpeg.org/ffmpeg.git \
	140fd653aed8cad774f991ba083e2d01e86420c7
get_source sources/llvm https://github.com/llvm/llvm-project.git \
	222fc11f2b8f25f6a0f4976272ef1bb7bf49521d

# We use our own recent Clang version,
# and Meson on Ubuntu 22.04 and older doesn't work with that
echo
echo "Getting meson..."
get_source pfx/meson https://github.com/mesonbuild/meson.git \
	415917b7c0d4759f73d0fe6564cbdda7fbf1eb9a
# We don't need to keep around git history
rm -rf pfx/meson/.git

# LLVM needs a fairly modern CMake,
# we need to compile on fairly old Linux distros
# to link against an old enough glibc ABI :(
echo
echo "Building CMake..."
mkdir -p build/cmake && cd build/cmake
"$TOP/sources/cmake/bootstrap" \
	--prefix="$PFX" \
	--parallel=$(nproc) \
	--generator=Ninja
nice ninja
ninja install
cd "$TOP"

export PATH="$PFX/bin:$PATH"

if [ "$os" = Darwin ]; then
	llvm_runtimes="libunwind;compiler-rt"
	llvm_build_sanitizers="OFF"
else
	llvm_runtimes="libunwind;libcxxabi;libcxx;compiler-rt"
	llvm_build_sanitizers="ON"
fi

echo
echo "Building LLVM..."
mkdir -p build/llvm && cd build/llvm
cmake -G Ninja \
	-DCMAKE_INSTALL_PREFIX="$PFX" \
	-DCMAKE_INSTALL_LIBDIR="$PFX/lib" \
	-DLIBCXX_INSTALL_LIBRARY_DIR="$PFX/lib" \
	-DLIBCXXABI_INSTALL_LIBRARY_DIR="$PFX/lib" \
	-DLIBUNWIND_INSTALL_LIBRARY_DIR="$PFX/lib" \
	-DCLANG_CONFIG_FILE_SYSTEM_DIR="../lib/clang" \
	-DCLANG_CONFIG_FILE_USER_DIR="../lib/clang" \
	-DLLVM_TARGETS_TO_BUILD="X86;AArch64" \
	-DCMAKE_BUILD_TYPE=Release \
	-DLLVM_ENABLE_PROJECTS="clang;lld" \
	-DLLVM_ENABLE_RUNTIMES="$llvm_runtimes" \
	-DLLVM_INSTALL_TOOLCHAIN_ONLY=ON \
	-DLLVM_INCLUDE_TESTS=OFF \
	-DLLVM_INCLUDE_BENCHMARKS=OFF \
	-DLLVM_INCLUDE_EXAMPLES=OFF \
	-DLLVM_INCLUDE_DOCS=OFF \
	-DCLANG_INCLUDE_TESTS=OFF \
	-DCLANG_INCLUDE_DOCS=OFF \
	-DLLVM_PARALLEL_LINK_JOBS=2 \
	-DLLVM_BUILD_STATIC=OFF \
	-DLLVM_BUILD_LLVM_DYLIB=ON \
	-DLLVM_LINK_LLVM_DYLIB=ON \
	-DCLANG_DEFAULT_CXX_STDLIB=libc++ \
	-DCLANG_DEFAULT_RTLIB=compiler-rt \
	-DCLANG_DEFAULT_LINKER=lld \
	-DCLANG_DEFAULT_UNWINDLIB=libunwind \
	-DCOMPILER_RT_BUILD_SANITIZERS="$llvm_build_sanitizers" \
	"$TOP/sources/llvm/llvm"
nice ninja clang lld
ninja install
cd "$TOP"

export CC="$PFX/bin/clang"
export CXX="$PFX/bin/clang++"
export LDFLAGS="-L$PFX/lib"

echo
echo "Building ffmpeg..."
mkdir -p build/ffmpeg && cd build/ffmpeg
"$TOP/sources/ffmpeg/configure" \
	--prefix="$PFX" \
	--libdir="$PFX/lib" \
	--enable-pic \
	--disable-static \
	--enable-shared \
	--disable-doc
nice make -j$(nproc)
make install
cd "$TOP"
