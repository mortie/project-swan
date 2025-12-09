#!/bin/sh

set -ex

cd "$(dirname "$0")"
TOP="$PWD"
PFX="$TOP/pfx"

echo
echo "Preparing prefix..."
rm -rf "$PFX"
mkdir -p "$PFX"

get_source() {
	if [ -e "$1/.checkout.stamp" ]; then
		return
	fi

	rm -rf "$1"
	git clone "$2" "$1"
	(cd "$1" && git checkout "$3" && git submodule update --init --recursive)
	touch "$1/.checkout.stamp"
}

echo
echo "Getting meson..."
get_source pfx/meson https://github.com/mesonbuild/meson.git \
	415917b7c0d4759f73d0fe6564cbdda7fbf1eb9a
rm -rf pfx/meson/.git
