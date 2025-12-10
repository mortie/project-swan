#!/bin/sh
set -e

SRCDIR="$(realpath "$(dirname "$0")")"

DATA_HOME="${XDG_DATA_HOME:-"$HOME/.local/share"}"
DESTDIR="$DATA_HOME/coffee.mort.Swan/swan"
SWAN_VERSION="$(cat "$SRCDIR/swan-version.txt")"

echo
echo "#"
echo "# SWAN Installer $SWAN_VERSION"
echo "#"

echo
echo "Enter install path [$DESTDIR]:"
printf '> '
read -r line
if [ -n "$line" ]; then
	echo 
	DESTDIR="$line"
fi

if [ "$SRCDIR" = "$DESTDIR" ]; then
	echo
	echo "Source and destination folders are the same."
	echo "Refusing to install."
	printf '> '
	read -r line
	exit 1
fi

if [ -e "$DESTDIR/worlds" ]; then
	echo
	echo "$DESTDIR contains a 'worlds' folder."
	echo "Refusing to overwrite."
	printf '> '
	read -r line
	exit 1
fi

if [ -e "$DESTDIR" ]; then
	if [ -e "$DESTDIR/swan-version.txt" ]; then
		echo
		echo "Existing SWAN install detected."
		echo "Old version: $(cat "$DESTDIR/swan-version.txt")"
		echo "New version: $SWAN_VERSION"
		echo "Overwrite? [y/N]"
		read -r line
		if [ "$line" != y ] && [ "$line" != Y ]; then
			echo "Aborting install."
			printf '> '
			read -r line
			exit 1
		fi
	else
		echo
		echo "$DESTDIR already exists and doesn't have a Swan install."
		echo "Refusing to overwrite."
		printf '> '
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

echo "Done! Press enter to continue."
printf '> '
read -r line
