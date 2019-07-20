#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <string.h>

#include "common.h"
#include "Tile.h"
#include "TileMap.h"

namespace Swan {

class Chunk {
public:
	int x_;
	int y_;
	bool dirty_ = false;
	Tile::ID tiles_[CHUNK_WIDTH][CHUNK_HEIGHT];
	sf::Texture texture_;
	sf::Sprite sprite_;

	Chunk(int x, int y): x_(x), y_(y) {
		texture_.create(CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE);
		sprite_ = sf::Sprite(texture_);
	}

	void setTile(TileMap &tmap, int x, int y, Tile::ID id) {
		tiles_[x][y] = id;
		drawBlock(tmap, x, y, id);
	}

	void drawBlock(int x, int y, Tile *t) {
		texture_.update(t->image_, x * TILE_SIZE, y * TILE_SIZE);
		dirty_ = true;
	}

	void drawBlock(TileMap &tmap, int x, int y, Tile::ID id) {
		drawBlock(x, y, tmap.get(id));
	}

	void redraw(TileMap &tmap);
	void fill(TileMap &tmap, Tile::ID id);
	void draw(Win &win);
};

}
