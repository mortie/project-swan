#include "Chunk.h"

#include <zlib.h>
#include <stdint.h>
#include <assert.h>

#include "log.h"
#include "Clock.h"
#include "gfxutil.h"
#include "World.h"
#include "Game.h"
#include "Win.h"

namespace Swan {

void Chunk::compress() {
	if (isCompressed())
		return;

	// We only need a fixed-length temp buffer;
	// if the compressed data gets too big, there's no point in compressing
	uint8_t dest[CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID)];

	uLongf destlen = sizeof(dest);
	int ret = compress2(
		(Bytef *)dest, &destlen,
		(Bytef *)data_.get(), DATA_SIZE,
		Z_BEST_COMPRESSION);

	if (ret == Z_OK) {
		data_.reset(new uint8_t[destlen]);
		memcpy(data_.get(), dest, destlen);

		texture_.reset();
		compressed_size_ = destlen;

		info
			<< "Compressed chunk " << pos_ << " from "
			<< DATA_SIZE << " bytes "
			<< "to " << destlen << " bytes";
	} else if (ret == Z_BUF_ERROR) {
		info
			<< "Didn't compress chunk " << pos_ << " "
			<< "because compressing it would've made it bigger";
	} else {
		warn << "Chunk compression error: " << ret << " (Out of memory?)";
	}
}

void Chunk::decompress() {
	if (!isCompressed())
		return;

	auto dest = std::make_unique<uint8_t[]>(DATA_SIZE);
	uLongf destlen = DATA_SIZE;
	int ret = uncompress(
		dest.get(), &destlen,
		(Bytef *)data_.get(), compressed_size_);

	if (ret != Z_OK) {
		panic << "Decompressing chunk failed: " << ret;
		abort();
	}

	data_ = std::move(dest);
	need_render_ = true;

	info
		<< "Decompressed chunk " << pos_ << " from "
		<< compressed_size_ << " bytes to "
		<< DATA_SIZE << " bytes.";
	compressed_size_ = -1;
}

void Chunk::renderLight(const Context &ctx, SDL_Renderer *rnd) {
	std::optional<RenderTarget> target;

	// The texture might not be created yet
	if (!light_texture_) {
		light_texture_.reset(SDL_CreateTexture(
			ctx.game.win_.renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
			CHUNK_WIDTH, CHUNK_HEIGHT));
		SDL_SetTextureBlendMode(light_texture_.get(), SDL_BLENDMODE_BLEND);

		target.emplace(rnd, texture_.get());
	} else {
		target.emplace(rnd, texture_.get());
	}

	// Fill light texture
	target.emplace(rnd, light_texture_.get());
	RenderBlendMode mode(rnd, SDL_BLENDMODE_NONE);
	RenderDrawColor color(rnd, 0, 0, 0, 0);
	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			int level = getLightLevel({ x, y });
			if (level >= 8)
				color.change(0, 0, 0, 0);
			else
				color.change(0, 0, 0, 255 - level * 32);
			SDL_Rect rect{ x, y, 1, 1 };
			SDL_RenderFillRect(rnd, &rect);
		}
	}

	need_light_render_ = false;
}

void Chunk::render(const Context &ctx, SDL_Renderer *rnd) {
	std::optional<RenderTarget> target;

	// The texture might not be created yet
	if (!texture_) {
		texture_.reset(SDL_CreateTexture(
			ctx.game.win_.renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
			CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE));
		SDL_SetTextureBlendMode(texture_.get(), SDL_BLENDMODE_BLEND);
		target.emplace(rnd, texture_.get());

		RenderBlendMode mode(rnd, SDL_BLENDMODE_NONE);
		RenderDrawColor color(rnd, 0, 0, 0, 0);
		SDL_Rect rect{ 0, 0, CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE };
		SDL_RenderFillRect(rnd, &rect);
	} else {
		target.emplace(rnd, texture_.get());
	}

	// We're caching tiles so we don't have to world.getTileByID() every time
	Tile::ID prevID = Tile::INVALID_ID;
	Tile *tile = ctx.game.invalid_tile_.get();

	// Fill tile texture
	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			Tile::ID id = getTileID(RelPos(x, y));
			if (id != prevID) {
				prevID = id;
				tile = &ctx.world.getTileByID(id);
			}

			SDL_Rect dest{x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
			SDL_RenderCopy(rnd, tile->image_.texture_.get(), nullptr, &dest);
		}
	}

	need_render_ = false;

	renderLight(ctx, rnd);
}

void Chunk::renderList(SDL_Renderer *rnd) {
	// Here, we know that the texture is created.
	// We still wanna render directly to the target texture
	RenderTarget target(rnd, texture_.get());

	// We must make sure the blend mode is NONE, because we want transparent
	// pixels to actually overwrite non-transparent pixels
	RenderBlendMode mode(rnd, SDL_BLENDMODE_NONE);

	// When we FillRect, we must fill transparency.
	RenderDrawColor color(rnd, 0, 0, 0, 0);

	for (auto &[pos, tex]: draw_list_) {
		SDL_Rect dest{pos.x * TILE_SIZE, pos.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
		SDL_RenderFillRect(rnd, &dest);
		SDL_RenderCopy(rnd, tex, nullptr, &dest);
	}
}

void Chunk::draw(const Context &ctx, Win &win) {
	if (isCompressed())
		return;

	// The world plane is responsible for managing initial renders
	if (need_render_)
		return;

	// We're responsible for the light level rendering though
	if (need_light_render_)
		renderLight(ctx, win.renderer_);

	if (draw_list_.size() > 0) {
		renderList(win.renderer_);
		draw_list_.clear();
	}

	auto chunkpos = pos_ * Vec2i(CHUNK_WIDTH, CHUNK_HEIGHT);
	SDL_Rect rect{ 0, 0, CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE };
	win.showTexture(chunkpos, texture_.get(), &rect);

	SDL_Rect texrect{ 0, 0, CHUNK_WIDTH, CHUNK_HEIGHT };
	win.showTexture(chunkpos, light_texture_.get(), &texrect, &rect);
}

Chunk::TickAction Chunk::tick(float dt) {
	assert(isActive());

	deactivate_timer_ -= dt;
	if (deactivate_timer_ <= 0) {
		if (is_modified_)
			return TickAction::DEACTIVATE;
		else
			return TickAction::DELETE;
	}

	return TickAction::NOTHING;
}

void Chunk::keepActive() {
	deactivate_timer_ = DEACTIVATE_INTERVAL;
	decompress();
}

}
