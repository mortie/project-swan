#!/bin/sh

set -ex

cd "$(dirname "$0")"
TOP="$PWD"
PFX="$TOP/pfx"
OUT="$TOP/out"

rm -rf "$OUT"
mkdir -p "$OUT"

# It's okay if $1 is empty, meson will use 'git describe'
SWAN_VERSION="$1"

echo "Building SWAN..."
mkdir -p build
rm -rf build/swan
"$PFX/meson/meson.py" setup \
	-Dprefix="$OUT" \
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

echo
echo "Fixing up dylib paths..."
for lib in \
	libcapnp-json.dylib \
	libcapnp.dylib \
	libcapnpc.dylib \
	libcpptoml.dylib \
	libcygnet.dylib \
	libfmt.dylib \
	libglfw.3.dylib \
	libglfw.dylib \
	libimgui.dylib \
	libkj.dylib \
	libportaudio.dylib \
	libscisasm.dylib \
	libscisavm.dylib \
	libsha1.dylib \
	libstb_image.dylib \
	libstb_truetype.dylib \
	libstb_vorbis.dylib \
	libswan.dylib
do
	install_name_tool -id "@rpath/$lib" "$OUT/lib/$lib"
	for f in "$OUT"/lib/* "$OUT"/bin/*; do
		install_name_tool -change "$OUT/lib/$lib" "@rpath/$lib" "$f"
	done
done

echo
echo "Compiling core mod..."
(cd "$OUT" && ./bin/swan-build core.mod .)
echo >>"$OUT/core.mod/mod.toml" "locked = true"
rm -rf "$OUT/core.mod/.swanbuild/obj"
rm -rf "$OUT/core.mod/.swanbuild/proto"
rm -rf "$OUT/core.mod/.swanbuild/swan.h.pch"

echo
echo "Creating launch script..."
cat >"$OUT/launch.command" <<'EOF'
#!/bin/sh
cd "$(dirname "$0")"
exec ./bin/swan-launcher
EOF
chmod +x "$OUT/launch.command"

echo
echo "Creating README..."
cat >"$OUT/README.txt" <<EOF
# Project: SWAN $SWAN_VERSION

Apple has made it hard to run software from people
who don't pay Apple a yearly developer fee.

To run Project: SWAN, do the following:

1. Open Terminal.app
2. Run the following command:

xattr -d -r com.apple.quarantine <path to launch.command>

3. Move this directory out of Downloads, to Desktop or Applications
4. Double-click launch.command

You can drag and drop launch.command into the Terminal window
instead of manually typing out its file path.

I will improve this at some point,
but improving packaging for macos is not a priority right now.
EOF
