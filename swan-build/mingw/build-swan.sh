#!/bin/sh

set -ex

cd "$(dirname "$0")"
TOP="$PWD"
PFX="$TOP/pfx"
OUT="$TOP/out"
MINGW_SYSROOT="/usr/x86_64-w64-mingw32/sys-root"

rm -rf "$OUT"
rm -rf build/swan build/native
mkdir -p "$OUT"

echo
echo "Building native swan-build..."
mkdir -p build
"$TOP/../../meson/meson.py" setup \
	-Dswan_cxx_path="x86_64-w64-mingw32-g++" \
	-Dswan_cxx_is_clang=false \
	-Dswan_cxx_is_mingw=true \
	-Dbuildtype=release \
	-Dprefix="$TOP/build/native/pfx" \
	build/native \
	../..
ninja -C build/native swan-build

echo
echo "Cross-building swan..."
mkdir -p build
meson setup \
	-Dprefix="$OUT" \
	-Dbuildtype=release \
	-Ddebug=true \
	-Dswan_version="$SWAN_VERSION" \
	-Dffmpeg=disabled \
	--cross-file=cross-mingw64.txt \
	build/swan \
	"../.."
nice ninja -C build/swan
ninja -C build/swan install

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
echo "Fixing up libraries..."
for lib in libglfw.dll libzlib_ng.dll; do
	# This seems to be literally nondeterministic??????
	if [ -e "$OUT/lib/$lib" ]; then
		mv "$OUT/lib/$lib" "$OUT/bin"
	fi
done

echo
echo "Grabbing mingw libraries..."
for lib in \
	libgcc_s_seh-1.dll \
	libwinpthread-1.dll
do
	cp "$MINGW_SYSROOT/mingw/bin/$lib" "$OUT/bin"
done

echo
echo "Compiling core mod..."
cd "$OUT"
"$TOP/build/native/swan-build" core.mod .
echo >>"core.mod/mod.toml" "locked = true"
rm -f bin/capnp bin/capnpc-c++
cd "$TOP"

echo
echo "Done!"
