#include "Chunk.h"

#include <zlib.h>
#include <stdint.h>
#include <assert.h>
#include <algorithm>

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
		compressedSize_ = destlen;

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
		(Bytef *)data_.get(), compressedSize_);

	if (ret != Z_OK) {
		panic << "Decompressing chunk failed: " << ret;
		abort();
	}

	data_ = std::move(dest);
	needRender_ = true;

	info
		<< "Decompressed chunk " << pos_ << " from "
		<< compressedSize_ << " bytes to "
		<< DATA_SIZE << " bytes.";
	compressedSize_ = -1;
}

void Chunk::renderLight(const Context &ctx, SDL_Renderer *rnd) {
	std::optional<RenderTarget> target;

	// The texture might not be created yet
	if (!lightTexture_) {
		lightTexture_.reset(SDL_CreateTexture(
			ctx.game.win_.renderer_, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET,
			CHUNK_WIDTH, CHUNK_HEIGHT));
		SDL_SetTextureBlendMode(lightTexture_.get(), SDL_BLENDMODE_BLEND);

		target.emplace(rnd, texture_.get());
	} else {
		target.emplace(rnd, texture_.get());
	}

	// Fill light texture
	target.emplace(rnd, lightTexture_.get());
	RenderBlendMode mode(rnd, SDL_BLENDMODE_NONE);
	RenderDrawColor color(rnd, 0, 0, 0, 0);
	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			int b = getLightLevel({ x, y });
			color.change(0, 0, 0, 255 - b);
			SDL_Rect rect{ x, y, 1, 1 };
			SDL_RenderFillRect(rnd, &rect);
		}
	}

	needLightRender_ = false;
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
	Tile *tile = ctx.game.invalidTile_.get();

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

	needRender_ = false;

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

	for (auto &[pos, tex]: drawList_) {
		SDL_Rect dest{pos.x * TILE_SIZE, pos.y * TILE_SIZE, TILE_SIZE, TILE_SIZE};
		SDL_RenderFillRect(rnd, &dest);
		SDL_RenderCopy(rnd, tex, nullptr, &dest);
	}
}

void Chunk::draw(const Context &ctx, Win &win) {
	if (isCompressed())
		return;

	// The world plane is responsible for managing initial renders
	if (needRender_)
		return;

	// We're responsible for the light level rendering though
	if (needLightRender_)
		renderLight(ctx, win.renderer_);

	if (drawList_.size() > 0) {
		renderList(win.renderer_);
		drawList_.clear();
	}

	auto chunkpos = pos_ * Vec2i(CHUNK_WIDTH, CHUNK_HEIGHT);
	SDL_Rect rect{ 0, 0, CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE };
	win.showTexture(chunkpos, texture_.get(), &rect);

	SDL_Rect texrect{ 0, 0, CHUNK_WIDTH, CHUNK_HEIGHT };
	win.showTexture(chunkpos, lightTexture_.get(), &texrect, &rect);
}

Chunk::TickAction Chunk::tick(float dt) {
	assert(isActive());

	deactivateTimer_ -= dt;
	if (deactivateTimer_ <= 0) {
		if (isModified_)
			return TickAction::DEACTIVATE;
		else
			return TickAction::DELETE;
	}

	return TickAction::NOTHING;
}

void Chunk::keepActive() {
	deactivateTimer_ = DEACTIVATE_INTERVAL;
	decompress();
}

}
