#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <string.h>

#include "common.h"
#include "Tile.h"
#include "TileMap.h"

namespace Swan {

class Chunk {
public:
	using ChunkPos = Vector2<int>;
	using RelPos = Vector2<int>;

	ChunkPos pos_;
	bool dirty_ = false;
	Tile::ID tiles_[CHUNK_WIDTH][CHUNK_HEIGHT];
	sf::Texture texture_;
	sf::Sprite sprite_;

	Chunk(ChunkPos pos): pos_(pos) {
		texture_.create(CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE);
		sprite_ = sf::Sprite(texture_);
	}

	void setTileID(TileMap &tmap, RelPos pos, Tile::ID id);
	Tile &getTile(TileMap &tmap, RelPos pos);
	void drawBlock(RelPos pos, const Tile &t);
	void drawBlock(TileMap &tmap, RelPos pos, Tile::ID id);
	void redraw(TileMap &tmap);
	void draw(Win &win);
};

}
