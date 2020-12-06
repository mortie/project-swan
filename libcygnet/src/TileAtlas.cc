#include "TileAtlas.h"

#include <iostream>
#include <vector>
#include <SDL_opengles2.h>
#include <stdio.h>
#include <string.h>
#include <swan-common/constants.h>

namespace Cygnet {

struct AtlasState {
	size_t tilesPerLine;
	size_t width = 0;
	size_t height = 0;

	std::vector<unsigned char> data;
};

TileAtlas::TileAtlas(): state_(std::make_unique<AtlasState>()) {
	GLint size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	state_->tilesPerLine = size / SwanCommon::TILE_SIZE;
}

TileAtlas::~TileAtlas() = default;

void TileAtlas::addTile(size_t tileId, const void *data, size_t len) {
	size_t rows = len / (SwanCommon::TILE_SIZE * 4);
	const unsigned char *bytes = (const unsigned char *)data;
	size_t x = tileId % state_->tilesPerLine;
	size_t y = tileId / state_->tilesPerLine;
	if (y >= state_->tilesPerLine) {
		std::cerr << "Cygnet: Warning: Tile ID " << tileId << " too big for texture atlas\n";
		return;
	}

	if (state_->width <= x) {
		state_->width = x + 1;
	}

	if (state_->height <= y) {
		state_->height = y + 1;
	}

	size_t tileImgSize = SwanCommon::TILE_SIZE * SwanCommon::TILE_SIZE * 4;
	size_t requiredSize = (x + 1) * (y + 1) * tileImgSize;
	state_->data.resize(requiredSize);

	for (size_t ty = 0; ty < rows; ++ty) {
		unsigned char *dest = state_->data.data() +
			((y + ty) * state_->width * SwanCommon::TILE_SIZE * 4) +
			(x * SwanCommon::TILE_SIZE * 4);
		const unsigned char *src = bytes + ty * SwanCommon::TILE_SIZE * 4;
		memcpy(dest, src, SwanCommon::TILE_SIZE * 4);
	}
}

const unsigned char *TileAtlas::getImage(size_t *w, size_t *h) {
	*w = state_->width * SwanCommon::TILE_SIZE;
	*h = state_->height * SwanCommon::TILE_SIZE;
	return state_->data.data();
}

}
