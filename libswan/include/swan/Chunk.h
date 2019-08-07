#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <string.h>
#include <stdint.h>
#include <memory>

#include "common.h"
#include "Tile.h"

namespace Swan {

class World;
class Game;

class Chunk {
public:
	using RelPos = TilePos;

	Chunk(ChunkPos pos): pos_(pos) {
		data_.reset(new uint8_t[CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID)]);
		visuals_.reset(new Visuals());
		visuals_->tex_.create(CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE);
		visuals_->sprite_ = sf::Sprite(visuals_->tex_);
		visuals_->dirty_ = false;
	}

	Tile::ID *getTileData();
	Tile::ID getTileID(RelPos pos);
	void setTileID(RelPos pos, Tile::ID id);
	void drawBlock(RelPos pos, const Tile &t);

	void compress();
	void decompress();
	void render(World &world);
	void draw(Game &game, Win &win);

	ChunkPos pos_;

private:
	static sf::Uint8 *renderbuf;

	std::unique_ptr<uint8_t[]> data_;

	int compressed_size_ = -1; // -1 if not compressed, a positive number if compressed
	bool need_render_ = false;

	struct Visuals {
		sf::Texture tex_;
		sf::Sprite sprite_;
		bool dirty_;
	};
	std::unique_ptr<Visuals> visuals_;
};

}
