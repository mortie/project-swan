#!/bin/sh

set -ex

cd "$(dirname "$0")"
TOP="$PWD"
PFX="$TOP/pfx"
OUT="$TOP/out"
MINGW_SYSROOT="/usr/x86_64-w64-mingw32/sys-root"

rm -rf "$OUT"
mkdir -p "$OUT"

echo
echo "Building native swan-build..."
mkdir -p build
rm -rf build/native
meson setup \
	-Dswan_cxx_path="x86_64-w64-mingw32-g++" \
	-Dswan_cxx_is_clang=false \
	-Dbuildtype=release \
	-Dprefix="$TOP/build/native/pfx" \
	-Dswan_dynlib_ext=".dll.a" \
	build/native \
	../..
ninja -C build/native swan-build

echo
echo "Cross-building swan..."
mkdir -p build
rm -rf build/swan
meson setup \
	-Dprefix="$OUT" \
	-Dbuildtype=release \
	-Ddebug=true \
	-Dswan_version="$SWAN_VERSION" \
	-Dffmpeg=disabled \
	--cross-file=cross-mingw64.txt \
	build/swan \
	"../.."
cd build/swan
nice ninja
ninja install
cd "$TOP"

echo
echo "Preparing native capnp..."

cat >"$OUT/bin/capnp" <<EOF
#!/bin/sh
exec "$TOP/build/swan/third-party/capnp/capnp_native" "\$@"
EOF
chmod +x "$OUT/bin/capnp"

cat >"$OUT/bin/capnpc-c++" <<EOF
#!/bin/sh
exec "$TOP/build/swan/third-party/capnp/capnpc-c++_native" "\$@"
EOF
chmod +x "$OUT/bin/capnpc-c++"

echo
echo "Compiling core mod..."
cd "$OUT"
"$TOP/build/native/swan-build" core.mod .
echo >>"core.mod/mod.toml" "locked = true"
rm -f bin/capnp bin/capnpc-c++
cd "$TOP"

echo
echo "Grabbing mingw libraries..."
for lib in \
	libgcc_s_seh-1.dll \
	libwinpthread-1.dll
do
	cp "$MINGW_SYSROOT/mingw/bin/$lib" "$OUT/bin"
done

echo
echo "Done!"
