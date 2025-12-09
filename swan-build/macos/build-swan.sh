#!/bin/sh

set -ex

cd "$(dirname "$0")"
TOP="$PWD"
PFX="$TOP/pfx"
OUT="$TOP/out"
APP="$OUT/Swan.app"

rm -rf "$OUT"
mkdir -p "$OUT"

SWAN_VERSION="$1"
if [ -z "$SWAN_VERSION" ]; then
	SWAN_VERSION="$(git describe --tags --always --dirty)+git"
fi

echo "Building SWAN..."
mkdir -p build
rm -rf build/swan
"$PFX/meson/meson.py" setup \
	-Dprefix="$APP/Contents" \
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
set +x
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
	install_name_tool -id "@rpath/$lib" "$APP/Contents/lib/$lib"
	for f in "$APP/Contents/lib"/* "$APP/Contents/bin/"*; do
		install_name_tool -change "$APP/Contents/lib/$lib" "@rpath/$lib" "$f"
	done
done
set -x

echo
echo "Compiling core mod..."
(cd "$APP/Contents" && ./bin/swan-build core.mod .)
echo >>"$APP/Contents/core.mod/mod.toml" "locked = true"
rm -rf "$APP/Contents/core.mod/.swanbuild/obj"
rm -rf "$APP/Contents/core.mod/.swanbuild/proto"
rm -rf "$APP/Contents/core.mod/.swanbuild/swan.h.pch"

echo
echo "Creating launch script..."
mkdir -p "$APP/Contents/MacOS"
cat >"$APP/Contents/MacOS/launch.sh" <<'EOF'
#!/bin/sh
cd "$(dirname "$0")/.." || return 1
exec ./bin/swan-launcher
EOF
chmod +x "$APP/Contents/MacOS/launch.sh"

echo
echo "Creating plist..."
cat >"$APP/Contents/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDisplayName</key>
	<string>Project: SWAN</string>
	<key>CFBundleExecutable</key>
	<string>launch.sh</string>
	<key>CFBundleIdentifier</key>
	<string>coffee.mort.Swan</string>
	<key>CFBundleName</key>
	<string>Project: SWAN</string>
	<key>CFBundleVersion</key>
	<string>$SWAN_VERSION</string>
	<key>CFBundleIconFile</key>
	<string>Swan</string>
	<key>NSHighResolutionCapable</key>
	<true/>
</dict>
</plist>
EOF

echo
echo "Create app icon..."
mkdir -p "$APP/Contents/Resources"
cp "$TOP/AppIcon.icns" "$APP/Contents/Resources/Swan.icns"

echo
echo "Creating README..."
cat >"$OUT/README.txt" <<EOF
# Project: SWAN $SWAN_VERSION

Apple has made it hard to run software from people
who don't pay Apple a yearly developer fee.

To run Project: SWAN, do the following:

1. Move this directory out of Downloads, for example to Applications (recommended)
2. Open Terminal.app
3. Run the following command:

xattr -d -r com.apple.quarantine <path to Swan.app>

4. Double-click Swan.app

You can drag and drop Swan.app into the Terminal window
instead of manually typing out its file path.
EOF
