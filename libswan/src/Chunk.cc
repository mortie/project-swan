#include "Chunk.h"

namespace Swan {

void Chunk::redraw(TileMap &tmap) {
	for (int x = 0; x < CHUNK_WIDTH; ++x) {
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			drawBlock(tmap, x, y, tiles_[x][y]);
		}
	}
}

void Chunk::draw(Win &win) {
	if (dirty_) {
		sprite_.setTexture(texture_);
		dirty_ = false;
	}

	win.setPos(Vec2(x_ * CHUNK_WIDTH, y_ * CHUNK_HEIGHT));
	win.draw(sprite_);
}

}
