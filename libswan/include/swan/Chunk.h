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

	// What does this chunk want the world gen to do after a tick?
	enum class TickAction {
		DEACTIVATE,
		DELETE,
		NOTHING,
	};

	Chunk(ChunkPos pos): pos_(pos) {
		data_.reset(new uint8_t[CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID)]);
	}

	Tile::ID *getTileData() {
		assert(isActive());
		return (Tile::ID *)data_.get();
	}

	Tile::ID getTileID(RelPos pos) {
		return getTileData()[pos.y * CHUNK_WIDTH + pos.x];
	}

	void setTileID(RelPos pos, Tile::ID id, SDL_Texture *tex) {
		getTileData()[pos.y * CHUNK_WIDTH + pos.x] = id;
		draw_list_.push_back({ pos, tex });
	}

	void setTileData(RelPos pos, Tile::ID id) {
		getTileData()[pos.y * CHUNK_WIDTH + pos.x] = id;
		need_render_ = true;
	}

	void render(const Context &ctx, SDL_Renderer *rnd);

	void compress();
	void decompress();
	void draw(const Context &ctx, Win &win);
	TickAction tick(float dt);

	bool isActive() { return deactivate_timer_ > 0; }
	void keepActive();
	void markModified() { is_modified_ = true; }

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
	bool is_modified_ = false;

	CPtr<SDL_Texture, SDL_DestroyTexture> texture_;
};

}
