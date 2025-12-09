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

install_lib() {
	cp "$PFX/lib/$1" "$OUT/lib/$1"
	patchelf --set-rpath '$ORIGIN' "$OUT/lib/$1"
}

echo "Compiling core mod..."
(cd "$OUT" && ./bin/swan-build core.mod .)
echo >>"$OUT/core.mod/mod.toml" "locked = true"
rm -rf "$OUT/core.mod/.swanbuild/obj"
rm -rf "$OUT/core.mod/.swanbuild/proto"
rm -rf "$OUT/core.mod/.swanbuild/swan.h.pch"

echo "Creating launch script..."
cat >"$OUT/launch.command" <<'EOF'
#!/bin/sh
cd "$(dirname "$0")"
exec ./bin/swan-launcher
EOF
chmod +x "$OUT/launch.command"
