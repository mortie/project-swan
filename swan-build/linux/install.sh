#!/bin/sh
set -e

SRCDIR="$(dirname "$0")"
DESTDIR="$HOME/.local/share/coffee.mort.Swan"

echo "Installing to '$DESTDIR'. Is this OK? [y/N]"
read -r line
if [ "$line" != y ] && [ "$line" != Y ]; then
	echo "Aborting install."
	read -r line
	exit 1
fi

if [ -e "$DESTDIR" ]; then
	echo "There's an existing install. Overwrite? (Save games will be preserved)"
	read -r line
	if [ "$line" != y ] && [ "$line" != Y ]; then
		echo "Aborting install."
		read -r line
		exit 1
	fi

	rm -rf "$DESTDIR"
fi

echo "Copying source folder..."
mkdir -p "$(dirname "$DESTDIR")"
cp -r "$SRCDIR" "$DESTDIR"

echo "Creating .desktop file..."
mkdir -p "$HOME/.local/share/applications"
cat >"$HOME/.local/share/applications/coffee.mort.Swan.desktop" <<END
[Desktop Entry]
Type=Application
Name=Project: SWAN
Path=$DESTDIR
Exec=$DESTDIR/bin/swan-launcher
Icon=$DESTDIR/assets/icon.png
Categories=Game
END

echo "Done! Press any key to continue."
read -r line
