#include "Chunk.h"

#include <zlib.h>
#include <stdint.h>

#include "log.h"
#include "Clock.h"
#include "gfxutil.h"
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

	SDL_Rect lockrect{ pos.x * TILE_SIZE, pos.y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
	TexLock lock(visuals_->texture_.get(), &lockrect);

	SDL_Rect srcrect{ 0, 0, t.image_.surface_->w, t.image_.surface_->h };
	SDL_Rect destrect{ 0, 0, TILE_SIZE, TILE_SIZE };
	lock.blit(&destrect, t.image_.surface_.get(), &srcrect);
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

	// We're caching tiles so we don't have to world.getTileByID() every time
	Tile::ID prevID = Tile::INVALID_ID;
	Tile *tile = ctx.game.invalid_tile_.get();

	// Locking the texture lets us write to its piexls
	TexLock lock(visuals_->texture_.get());

	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			Tile::ID id = getTileID(RelPos(x, y));
			if (id != prevID) {
				prevID = id;
				tile = &ctx.world.getTileByID(id);
			}

			// Find the source surface and rect...
			auto &srcsurf = tile->image_.surface_;
			SDL_Rect srcrect{ 0, 0, srcsurf->w, srcsurf->h };

			// ...and blit it to the appropriate place
			SDL_Rect destrect{ x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE };
			if (lock.blit(&destrect, srcsurf.get(), &srcrect) < 0)
				warn << "Failed to blit surface: " << SDL_GetError();
		}
	}
}

void Chunk::draw(const Context &ctx, Win &win) {
	if (isCompressed())
		return;

	if (need_render_) {
		render(ctx);
		need_render_ = false;
	}

	SDL_Rect rect{ 0, 0, CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE };
	win.showTexture(
		pos_ * Vec2i(CHUNK_WIDTH, CHUNK_HEIGHT),
		visuals_->texture_.get(), &rect);
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
