#!/bin/sh

set -ex

cd "$(dirname "$0")"
TOP="$PWD"
PFX="$TOP/pfx"
OUT="$TOP/out"

rm -rf "$OUT"
mkdir -p "$OUT"

SWAN_VERSION="$1"
if [ -z "$SWAN_VERSION" ]; then
	SWAN_VERSION="$(git describe --tags --always --dirty)+git"
fi

export CC="$PFX/bin/clang"
export CXX="$PFX/bin/clang++"
export PKG_CONFIG_PATH="$PFX/lib/pkgconfig"
export LD_LIBRARY_PATH="$PFX/lib"
export LDFLAGS="-L$PFX/lib"
export PATH="$PFX/bin:$PATH"
export CAPNP_SYSROOT="$PFX/include"
export PATH="$PFX/bin:$PATH"

echo
echo "Building SWAN..."
mkdir -p build
rm -rf build/swan
"$PFX/meson/meson.py" setup \
	-Dprefix="$OUT" \
	-Dclangxx_path="./bin/clang++" \
	-Dbuildtype=release \
	-Ddebug=true \
	-Dswan_version="$SWAN_VERSION" \
	-Dffmpeg=disabled \
	build/swan \
	"../.."
cd build/swan
nice ninja
ninja install
cd "$TOP"

install_lib() {
	cp "$PFX/lib/$1" "$OUT/lib/$1"
	patchelf --set-rpath '$ORIGIN' "$OUT/lib/$1"
}

echo
echo "Copying over libraries..."
cp "$PFX/lib/libc++.so" "$OUT/lib/"
install_lib libc++.so.1
install_lib libc++abi.so.1
install_lib libunwind.so.1
install_lib libclang-cpp.so.21.1
install_lib libLLVM.so.21.1
ln -s libc++abi.so.1 "$OUT/lib/libc++abi.so"
ln -s libunwind.so.1 "$OUT/lib/libunwind.so"
cp -r "$PFX/lib/clang" "$OUT/lib/"

echo
echo "Copying over headers..."
cp -r "$PFX/include"/* "$OUT/include/"

echo
echo "Copying over binaries..."
cp "$PFX/bin"/clang "$OUT/bin/"
ln -s clang "$OUT/bin/clang++"
cp "$PFX/bin/ld.lld" "$OUT/bin/"

echo
echo "Compiling core mod..."
(cd "$OUT" && ./bin/swan-build core.mod .)
echo >>"$OUT/core.mod/mod.toml" "locked = true"
rm -rf "$OUT/core.mod/.swanbuild/obj"
rm -rf "$OUT/core.mod/.swanbuild/proto"
rm -rf "$OUT/core.mod/.swanbuild/swan.h.pch"

echo
echo "Creating launch script..."
cat >"$OUT/launch.sh" <<'EOF'
#!/bin/sh
cd "$(dirname "$0")"
exec ./bin/swan-launcher
EOF
chmod +x "$OUT/launch.sh"

echo
echo "Creating install script..."
cp "$TOP/install.sh" "$OUT/install.sh"
chmod +x "$OUT/install.sh"

echo
echo "Creating swan-version.txt..."
echo "$SWAN_VERSION" >"$OUT/swan-version.txt"
