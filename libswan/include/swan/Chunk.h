#pragma once

#include <string.h>
#include <stdint.h>
#include <memory>

#include "util.h"
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
	}

	Tile::ID *getTileData();
	Tile::ID getTileID(RelPos pos);
	void setTileID(RelPos pos, Tile::ID id, SDL_Texture *tex);
	void setTileData(RelPos pos, Tile::ID id);
	void render(const Context &ctx, SDL_Renderer *rnd);

	void compress();
	void decompress();
	void draw(const Context &ctx, Win &win);
	void tick(float dt);

	bool keepActive(); // Returns true if chunk was inactive
	bool isActive() { return deactivate_timer_ > 0; }

	ChunkPos pos_;

private:
	static constexpr float DEACTIVATE_INTERVAL = 20;
	static uint8_t *renderbuf;

	void renderList(SDL_Renderer *rnd);

	bool isCompressed() { return compressed_size_ != -1; }

	std::unique_ptr<uint8_t[]> data_;
	std::vector<std::pair<RelPos, SDL_Texture *>> draw_list_;

	ssize_t compressed_size_ = -1; // -1 if not compressed, a positive number if compressed
	bool need_render_ = false;
	float deactivate_timer_ = DEACTIVATE_INTERVAL;

	CPtr<SDL_Texture, SDL_DestroyTexture> texture_;
};

}
