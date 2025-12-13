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

SWAN_VERSION="$1"
if [ -z "$SWAN_VERSION" ]; then
	SWAN_VERSION="$(git describe --tags --always --dirty)+git"
fi

echo
echo "Building native swan-build..."
mkdir -p build
../../meson/meson.py setup \
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
../../meson/meson.py setup \
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
# Some DLLs end up in lib/ for some reason?
mv "$OUT/lib"/*.dll "$OUT"/bin 2>/dev/null || true

echo
echo "Compiling core mod..."
cd "$OUT"
"$TOP/build/native/swan-build" core.mod .
echo >>"core.mod/mod.toml" "locked = true"
rm -f bin/capnp bin/capnpc-c++
cd "$TOP"

echo
echo "Creating launch script..."
cat >"$OUT/launch.bat" <<'EOF'
bin\swan-launcher
EOF

echo
echo "Done!"
