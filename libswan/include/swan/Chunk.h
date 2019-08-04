#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <string.h>

#include "common.h"
#include "Tile.h"

namespace Swan {

class World;

class Chunk {
public:
	using RelPos = TilePos;

	Chunk(ChunkPos pos): pos_(pos) {
		texture_.create(CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE);
		sprite_ = sf::Sprite(texture_);
	}

	void setTileID(World &world, RelPos pos, Tile::ID id);
	Tile &getTile(World &world, RelPos pos);
	void redraw(World &world);
	void draw(Win &win);

	ChunkPos pos_;
	Tile::ID tiles_[CHUNK_WIDTH][CHUNK_HEIGHT];

private:
	void drawBlock(RelPos pos, const Tile &t);
	void drawBlock(World &world, RelPos pos, Tile::ID id);
	bool dirty_ = false;
	sf::Texture texture_;
	sf::Sprite sprite_;
};

}
