#include "Chunk.h"

#include <string.h>

void Chunk::clear() {
	memset(tiles_, 0, sizeof(tiles_));
}

void Chunk::draw(Win &win) {
}
