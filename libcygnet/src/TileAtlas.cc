#include "TileAtlas.h"

#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <swan-common/constants.h>

#include "gl.h"

namespace Cygnet {

struct AtlasState {
	size_t tilesPerLine;
	size_t width = 0;
	size_t height = 0;

	std::vector<unsigned char> data;
};

TileAtlas::TileAtlas(): state_(std::make_unique<AtlasState>())
{
	GLint size;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &size);
	state_->tilesPerLine = std::min(size / SwanCommon::TILE_SIZE, 256);
}

TileAtlas::TileAtlas(TileAtlas &&) = default;
TileAtlas::~TileAtlas() = default;

void TileAtlas::addTile(size_t tileId, const void *data)
{
	const unsigned char *bytes = (const unsigned char *)data;
	size_t x = tileId % state_->tilesPerLine;
	size_t y = tileId / state_->tilesPerLine;

	if (state_->width <= x) {
		state_->width = x + 1;
	}

	if (state_->height <= y) {
		state_->height = y + 1;
	}

	size_t requiredSize = state_->tilesPerLine * SwanCommon::TILE_SIZE *
		state_->height * SwanCommon::TILE_SIZE * 4;
	state_->data.resize(requiredSize);

	for (size_t ty = 0; ty < SwanCommon::TILE_SIZE; ++ty) {
		const unsigned char *src = bytes + ty * SwanCommon::TILE_SIZE * 4;
		unsigned char *dest = state_->data.data() +
			(y * SwanCommon::TILE_SIZE + ty) * state_->tilesPerLine * SwanCommon::TILE_SIZE * 4 +
			(x * SwanCommon::TILE_SIZE * 4);
		memcpy(dest, src, SwanCommon::TILE_SIZE * 4);
	}
}

const unsigned char *TileAtlas::getImage(size_t *w, size_t *h)
{
	*w = state_->tilesPerLine * SwanCommon::TILE_SIZE;
	*h = state_->height * SwanCommon::TILE_SIZE;
	return state_->data.data();
}

}
