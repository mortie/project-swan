#include "Chunk.h"

#include "World.h"

namespace Swan {

void Chunk::setTileID(World &world, RelPos pos, Tile::ID id) {
	tiles_[pos.x_][pos.y_] = id;
	drawBlock(world, pos, id);
}

Tile &Chunk::getTile(World &world, RelPos pos) {
	return world.getTileByID(tiles_[pos.x_][pos.y_]);
}

void Chunk::drawBlock(RelPos pos, const Tile &t) {
	texture_.update(t.	image_, pos.x_ * TILE_SIZE, pos.y_ * TILE_SIZE);
	dirty_ = true;
}

void Chunk::drawBlock(World &world, RelPos pos, Tile::ID id) {
	drawBlock(pos, world.getTileByID(id));
}

void Chunk::redraw(World &world) {
	for (int x = 0; x < CHUNK_WIDTH; ++x) {
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			drawBlock(world, ChunkPos(x, y), tiles_[x][y]);
		}
	}
}

void Chunk::draw(Win &win) {
	if (dirty_) {
		sprite_.setTexture(texture_);
		dirty_ = false;
	}

	win.setPos(pos_ * Vec2i(CHUNK_WIDTH, CHUNK_HEIGHT));
	win.draw(sprite_);
}

}
