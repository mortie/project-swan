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
	Tile::TileID tiles_[CHUNK_WIDTH][CHUNK_HEIGHT];
	sf::Texture texture_;
	sf::Sprite sprite_;

	Chunk(int x, int y): x_(x), y_(y) {
		texture_.create(CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE);
		sprite_ = sf::Sprite(texture_);
	}

	void setTile(TileMap &tmap, int x, int y, Tile::TileID id) {
		tiles_[x][y] = id;
		drawBlock(tmap, x, y, id);
	}

	void drawBlock(TileMap &tmap, int x, int y, Tile::TileID id) {
		Tile *t = tmap.get(id);
		fprintf(stderr, "Drawing %s to %i,%i in chunk %i,%i\n", t->name_.c_str(), x, y, x_, y_);
		texture_.update(t->image_, x * TILE_SIZE, y * TILE_SIZE);
		dirty_ = true;
	}

	void drawBlock(TileMap &tmap, int x, int y) {
		drawBlock(tmap, x, y, tiles_[x][y]);
	}

	void clear() {
		memset(tiles_, 0, sizeof(tiles_));
	}

	void draw(Win &win) {
		if (dirty_) {
			sprite_.setTexture(texture_);
			dirty_ = false;
		}

		win.setPos(Vec2(x_ * CHUNK_WIDTH, y_ * CHUNK_HEIGHT));
		win.draw(sprite_);
	}
};

}
