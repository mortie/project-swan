#include "Chunk.h"

#include "World.h"

namespace Swan {

sf::Uint8 *Chunk::imgbuf = new sf::Uint8[CHUNK_WIDTH * TILE_SIZE * CHUNK_HEIGHT * TILE_SIZE * 4];

void Chunk::setTileID(World &world, RelPos pos, Tile::ID id) {
	tiles_[pos.x_][pos.y_] = id;
	drawBlock(world, pos, id);
}

Tile &Chunk::getTile(World &world, RelPos pos) {
	return world.getTileByID(tiles_[pos.x_][pos.y_]);
}

void Chunk::drawBlock(RelPos pos, const Tile &t) {
	texture_.update(t.image_, pos.x_ * TILE_SIZE, pos.y_ * TILE_SIZE);
	dirty_ = true;
}

void Chunk::drawBlock(World &world, RelPos pos, Tile::ID id) {
	drawBlock(pos, world.getTileByID(id));
}

void Chunk::render(World &world) {
	Tile::ID prevID = Tile::INVALID_ID;
	Tile &tile = Tile::INVALID_TILE;

	for (int x = 0; x < CHUNK_WIDTH; ++x) {
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			Tile::ID id = tiles_[x][y];
			if (id != prevID) {
				prevID = id;
				tile = world.getTileByID(id);
			}

			const sf::Uint8 *imgptr = tile.image_.getPixelsPtr();
			for (int imgy = 0; imgy < TILE_SIZE; ++imgy) {
				int pixx = x * TILE_SIZE;
				int pixy = y * TILE_SIZE + imgy;
				sf::Uint8 *pix = imgbuf +
					pixy * CHUNK_WIDTH * TILE_SIZE * 4 +
					pixx * 4;
				memcpy(pix, imgptr + imgy * TILE_SIZE * 4, TILE_SIZE * 4);
			}
		}
	}

	texture_.update(imgbuf, CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE, 0, 0);
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
