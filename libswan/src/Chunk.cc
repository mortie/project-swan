#include "Chunk.h"

#include <string.h>

namespace Swan {

void Chunk::clear() {
	memset(tiles_, 0, sizeof(tiles_));
}

void Chunk::draw(Win &win) {
}

}
