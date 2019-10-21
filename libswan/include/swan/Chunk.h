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

	ChunkPos pos_;

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
	void render(const Context &ctx);
	void draw(const Context &ctx, Win &win);
	void tick(float dt);

	bool keepActive(); // Returns true if chunk was inactive
	bool isActive() { return deactivate_timer_ > 0; }

private:
	static constexpr float DEACTIVATE_TIME = 20;
	static sf::Uint8 *renderbuf;

	bool isCompressed() { return compressed_size_ != -1; }

	std::unique_ptr<uint8_t[]> data_;

	ssize_t compressed_size_ = -1; // -1 if not compressed, a positive number if compressed
	bool need_render_ = false;
	float deactivate_timer_ = DEACTIVATE_TIME;

	struct Visuals {
		sf::Texture tex_;
		sf::Sprite sprite_;
		bool dirty_;
	};
	std::unique_ptr<Visuals> visuals_;
};

}
