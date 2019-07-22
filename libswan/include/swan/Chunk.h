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

	void setTileID(TileMap &tmap, RelPos pos, Tile::ID id) {
		tiles_[pos.x_][pos.y_] = id;
		drawBlock(tmap, pos, id);
	}

	Tile &getTile(TileMap &tmap, RelPos pos) {
		return tmap.get(tiles_[pos.x_][pos.y_]);
	}

	void drawBlock(RelPos pos, const Tile &t) {
		texture_.update(t.image_, pos.x_ * TILE_SIZE, pos.y_ * TILE_SIZE);
		dirty_ = true;
	}

	void drawBlock(TileMap &tmap, RelPos pos, Tile::ID id) {
		drawBlock(pos, tmap.get(id));
	}

	void redraw(TileMap &tmap) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			for (int y = 0; y < CHUNK_HEIGHT; ++y) {
				drawBlock(tmap, ChunkPos(x, y), tiles_[x][y]);
			}
		}
	}

	void draw(Win &win);
};

}
