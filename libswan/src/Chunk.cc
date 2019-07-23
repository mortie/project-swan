#include "Chunk.h"

namespace Swan {

void Chunk::setTileID(TileMap &tmap, RelPos pos, Tile::ID id) {
	tiles_[pos.x_][pos.y_] = id;
	drawBlock(tmap, pos, id);
}

Tile &Chunk::getTile(TileMap &tmap, RelPos pos) {
	return tmap.get(tiles_[pos.x_][pos.y_]);
}

void Chunk::drawBlock(RelPos pos, const Tile &t) {
	texture_.update(t.image_, pos.x_ * TILE_SIZE, pos.y_ * TILE_SIZE);
	dirty_ = true;
}

void Chunk::drawBlock(TileMap &tmap, RelPos pos, Tile::ID id) {
	drawBlock(pos, tmap.get(id));
}

void Chunk::redraw(TileMap &tmap) {
	for (int x = 0; x < CHUNK_WIDTH; ++x) {
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			drawBlock(tmap, ChunkPos(x, y), tiles_[x][y]);
		}
	}
}

void Chunk::draw(Win &win) {
	if (dirty_) {
		sprite_.setTexture(texture_);
		dirty_ = false;
	}

	win.setPos(Vec2(pos_.x_ * CHUNK_WIDTH, pos_.y_ * CHUNK_HEIGHT));
	win.draw(sprite_);
}

}
