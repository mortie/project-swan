#!/bin/sh

set -e

get_out() {
    shift
    while [ $# -gt 0 ]; do
        if ! echo "$1" | grep '^-o' >/dev/null; then
            shift
            continue
        fi

        if [ "$1" = "-o" ]; then
            echo "$2"
        else
            echo "$1" | sed 's/^-o//'
        fi

        return
    done
}

out="$(get_out "$@" | sed 's#/#__#g')"
if [ -z "$out" ]; then
    exec "$@"
else
    mkdir -p build-times
    before="$(date +%s%N)"
    "$@"
    after="$(date +%s%N)"
    echo "$((after - before)) $before $after" >"build-times/$out.time"
fi
