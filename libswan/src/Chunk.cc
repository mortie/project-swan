#include "Chunk.h"

#include <zlib.h>
#include <stdint.h>

#include "log.h"
#include "World.h"
#include "Game.h"
#include "Win.h"

namespace Swan {

uint8_t *Chunk::renderbuf = new uint8_t[CHUNK_WIDTH * TILE_SIZE * CHUNK_HEIGHT * TILE_SIZE * 4];

Tile::ID *Chunk::getTileData() {
	keepActive();
	return (Tile::ID *)data_.get();
}

Tile::ID Chunk::getTileID(RelPos pos) {
	return getTileData()[pos.y * CHUNK_WIDTH + pos.x];
}

void Chunk::setTileID(RelPos pos, Tile::ID id) {
	getTileData()[pos.y * CHUNK_WIDTH + pos.x] = id;
}

void Chunk::drawBlock(RelPos pos, const Tile &t) {
	keepActive();

	//visuals_->tex_.update(*t.image_, pos.x * TILE_SIZE, pos.y * TILE_SIZE);
	visuals_->dirty_ = true;
}

void Chunk::compress() {
	if (isCompressed())
		return;

	// We only need a fixed-length temp buffer;
	// if the compressed data gets too big, there's no point in compressing
	uint8_t dest[CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID)];

	uLongf destlen = sizeof(dest);
	int ret = compress2(
			(Bytef *)dest, &destlen,
			(Bytef *)data_.get(), CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID),
			Z_BEST_COMPRESSION);

	if (ret == Z_OK) {
		data_.reset(new uint8_t[destlen]);
		memcpy(data_.get(), dest, destlen);

		visuals_.reset();
		compressed_size_ = destlen;

		info
			<< "Compressed chunk " << pos_ << " from "
			<< CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID) << " bytes "
			<< "to " << destlen << " bytes.";
	} else if (ret == Z_BUF_ERROR) {
		info
			<< "Didn't compress chunk " << pos_ << " "
			<< "because compressing it would've made it bigger.";
	} else {
		warn << "Chunk compression error: " << ret << " (Out of memory?)";
	}
}

void Chunk::decompress() {
	if (!isCompressed())
		return;

	auto dest = std::make_unique<uint8_t[]>(CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID));
	uLongf destlen = CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID);
	int ret = uncompress(
			dest.get(), &destlen,
			(Bytef *)data_.get(), compressed_size_);

	if (ret != Z_OK) {
		panic << "Decompressing chunk failed: " << ret;
		abort();
	}

	data_ = std::move(dest);

	visuals_.reset(new Visuals());
	visuals_->dirty_ = true;
	need_render_ = true;

	info
		<< "Decompressed chunk " << pos_ << " from "
		<< compressed_size_ << " bytes to "
		<< CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID) << " bytes.";
	compressed_size_ = -1;
}

void Chunk::render(const Context &ctx) {
	// The texture might not be created yet
	if (!visuals_->texture_) {
		visuals_->texture_.reset(SDL_CreateTexture(
			ctx.game.win_.renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING,
			CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE));
	}

	Tile::ID prevID = Tile::INVALID_ID;
	Tile *tile = ctx.game.invalid_tile_.get();

	SDL_Rect rect{ 0, 0, CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE };
	uint8_t *pixels;
	int pitch;
	if (SDL_LockTexture(visuals_->texture_.get(), &rect, (void **)&pixels, &pitch) < 0) {
		panic << "Failed to lock texture: " << SDL_GetError();
		abort();
	}
	auto lock = makeDeferred([this] { SDL_UnlockTexture(visuals_->texture_.get()); });

	for (int x = 0; x < CHUNK_WIDTH; ++x) {
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			Tile::ID id = getTileID(RelPos(x, y));
			if (id != prevID) {
				prevID = id;
				tile = &ctx.world.getTileByID(id);
			}

			auto &tilesurf = tile->image_.surface_;

			for (int imgy = 0; imgy < TILE_SIZE; ++imgy) {
				uint8_t *tilepix = (uint8_t *)tilesurf->pixels + imgy * tilesurf->pitch;
				uint8_t *destpix = pixels + y * pitch + (x * TILE_SIZE) * 4;
				memcpy(destpix, tilepix, TILE_SIZE * 4);
			}
		}
	}

	visuals_->dirty_ = true;
}

void Chunk::draw(const Context &ctx, Win &win) {
	if (isCompressed())
		return;

	if (need_render_) {
		render(ctx);
		need_render_ = false;
	}

	if (visuals_->dirty_) {
		//visuals_->sprite_.setTexture(visuals_->tex_);
		visuals_->dirty_ = false;
	}

	SDL_Rect rect{ 0, 0, CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE };
	win.showTexture(pos_, visuals_->texture_.get(), &rect);
	//win.draw(visuals_->sprite_);
}

void Chunk::tick(float dt) {
	if (deactivate_timer_ <= 0)
		return;

	deactivate_timer_ -= dt;
	if (deactivate_timer_ <= 0) {
		compress();
	}
}

bool Chunk::keepActive() {
	bool wasActive = isActive();
	deactivate_timer_ = DEACTIVATE_INTERVAL;

	if (wasActive)
		return false;

	decompress();
	return true;
}

}
