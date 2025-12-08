#!/bin/sh

set -ex

cd "$(dirname "$0")"
TOP="$PWD"
PFX="$TOP/pfx"
OUT="$TOP/out"

rm -rf "$OUT"
mkdir -p "$OUT"

export CC="$PFX/bin/clang"
export CXX="$PFX/bin/clang++"
export PKG_CONFIG_PATH="$PFX/lib/pkgconfig"
export LD_LIBRARY_PATH="$PFX/lib"
export LDFLAGS="-L$PFX/lib"
export PATH="$PFX/bin:$PATH"
export CAPNP_SYSROOT="$PFX/include"
export PATH="$PFX/bin:$PATH"

echo "Building SWAN..."
mkdir -p build
rm -rf build/swan
"$PFX/meson/meson.py" setup \
	-Dprefix="$OUT" \
	-Dclangxx_path="./bin/clang++" \
	-Dbuildtype=release \
	-Ddebug=true \
	build/swan \
	".."
cd build/swan
nice ninja
ninja install
cd "$TOP"

install_lib() {
	cp "$PFX/lib/$1" "$OUT/lib/$1"
	patchelf --set-rpath '$ORIGIN' "$OUT/lib/$1"
}

echo "Copying over libraries..."
cp "$PFX/lib/libc++.so" "$OUT/lib/"
install_lib libc++.so.1
install_lib libc++abi.so.1
install_lib libunwind.so.1
install_lib libavcodec.so.62
install_lib libavdevice.so.62
install_lib libavfilter.so.11
install_lib libavformat.so.62
install_lib libavutil.so.60
install_lib libswresample.so.6
install_lib libswscale.so.9
install_lib libclang-cpp.so.21.1
install_lib libLLVM.so.21.1
ln -s libc++abi.so.1 "$OUT/lib/libc++abi.so"
ln -s libunwind.so.1 "$OUT/lib/libunwind.so"
cp -r "$PFX/lib/clang" "$OUT/lib/"

echo "Copying over headers..."
cp -r "$PFX/include"/* "$OUT/include/"
rm -rf "$OUT/include/libavcodec"
rm -rf "$OUT/include/libavdevice"
rm -rf "$OUT/include/libavfilter"
rm -rf "$OUT/include/libavformat"
rm -rf "$OUT/include/libavutil"
rm -rf "$OUT/include/libswresample"
rm -rf "$OUT/include/libswscale"

echo "Copying over binaries..."
cp "$PFX/bin"/clang "$OUT/bin/"
ln -s clang "$OUT/bin/clang++"
cp "$PFX/bin/ld.lld" "$OUT/bin/"

echo "Compiling core mod..."
(cd "$OUT" && ./bin/swan-build core.mod .)
echo >>"$OUT/core.mod/mod.toml" "locked = true"

echo "Creating launch script..."
cat >"$OUT/launch.sh" <<'EOF'
#!/bin/sh
cd "$(dirname "$0")"
exec ./bin/swan-launcher
EOF
chmod +x "$OUT/launch.sh"

echo "Creating install script..."
cat >"$OUT/install.sh" <<'EOF'
#!/bin/sh
set -e

SRCDIR="$(dirname "$0")"
DESTDIR="$HOME/.local/opt/swan"

echo "Installing to $DESTDIR. Is this OK? [y/N]"
read -r line
if [ "$line" != y ] && [ "$line" != Y ]; then
	echo "Aborting install."
	exit 1
fi

if [ -e "$DESTDIR" ]; then
	echo "There's an existing install. Overwrite? (Save games will be preserved)"
	read -r line
	if [ "$line" != y ] && [ "$line" != Y ]; then
		echo "Aborting install."
		exit 1
	fi

	rm -rf "$DESTDIR.oldsaves~"
	if [ -e "$DESTDIR/worlds" ]; then
		cp -r "$DESTDIR/worlds" "$DESTDIR.oldworlds~"
	fi
	rm -rf "$DESTDIR"
fi

echo "Copying source folder..."
mkdir -p "$(dirname "$DESTDIR")"
cp -r "$SRCDIR" "$DESTDIR"

for x in "$DESTDIR.oldworlds~"/*; do
	if [ -e "$x" ]; then
		rm -rf "$DESTDIR/worlds/$(basename "$x")"
		mv "$x" "$DESTDIR/worlds/"
	fi
done
rm -rf "$DESTDIR.oldworlds~"

echo "Creating .desktop file..."
mkdir -p "$HOME/.local/share/applications"
cat >"$HOME/.local/share/applications/swan.desktop" <<END
[Desktop Entry]
Type=Application
Name=Project: SWAN
Exec=$DESTDIR/launch.sh
Icon=$DESTDIR/assets/icon.png
Categories=Game
END

echo "Done!"
EOF
chmod +x "$OUT/install.sh"
