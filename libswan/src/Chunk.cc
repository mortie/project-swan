#include "Chunk.h"

#include <SFML/System/Clock.hpp>
#include <zlib.h>

#include "World.h"
#include "Game.h"

namespace Swan {

sf::Uint8 *Chunk::renderbuf = new sf::Uint8[CHUNK_WIDTH * TILE_SIZE * CHUNK_HEIGHT * TILE_SIZE * 4];

Tile::ID *Chunk::getTileData() {
	if (compressed_size_ != -1)
		decompress();

	return (Tile::ID *)data_.get();
}

Tile::ID Chunk::getTileID(RelPos pos) {
	return getTileData()[pos.y_ * CHUNK_WIDTH + pos.x_];
}

void Chunk::setTileID(RelPos pos, Tile::ID id) {
	getTileData()[pos.y_ * CHUNK_WIDTH + pos.x_] = id;
}

void Chunk::drawBlock(RelPos pos, const Tile &t) {
	if (compressed_size_ != -1)
		decompress();

	visuals_->tex_.update(*t.image, pos.x_ * TILE_SIZE, pos.y_ * TILE_SIZE);
	visuals_->dirty_ = true;
}

void Chunk::compress() {
	if (compressed_size_ != -1)
		return;

	sf::Clock clock;

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

		fprintf(stderr, "Compressed chunk %i,%i from %lu bytes to %lu bytes in %.3fs.\n",
				pos_.x_, pos_.y_, CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID), destlen,
				clock.getElapsedTime().asSeconds());
	} else if (ret == Z_BUF_ERROR) {
		fprintf(stderr, "Didn't compress chunk %i,%i because compressing it would've made it bigger.\n",
				pos_.x_, pos_.y_);
	} else {
		fprintf(stderr, "Chunk compression error: %i (Out of memory?)\n", ret);
	}
}

void Chunk::decompress() {
	uint8_t *dest = new uint8_t[CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID)];
	uLongf destlen = CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID);
	int ret = uncompress(
			dest, &destlen,
			(Bytef *)data_.get(), compressed_size_);

	if (ret != Z_OK) {
		fprintf(stderr, "Decompressing chunk failed: %i\n", ret);
		delete[] dest;
		abort();
	}

	data_.reset(dest);
	visuals_.reset(new Visuals());
	visuals_->tex_.create(CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE);
	visuals_->sprite_ = sf::Sprite();
	visuals_->dirty_ = true;
	need_render_ = true;
	compressed_size_ = -1;
}

void Chunk::render(const Context &ctx) {
	Tile::ID prevID = Tile::INVALID_ID;
	Tile *tile = &Tile::INVALID_TILE;

	for (int x = 0; x < CHUNK_WIDTH; ++x) {
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			Tile::ID id = getTileID(RelPos(x, y));
			if (id != prevID) {
				prevID = id;
				tile = &ctx.world.getTileByID(id);
			}

			const sf::Uint8 *imgptr = tile->image->getPixelsPtr();
			for (int imgy = 0; imgy < TILE_SIZE; ++imgy) {
				int pixx = x * TILE_SIZE;
				int pixy = y * TILE_SIZE + imgy;
				sf::Uint8 *pix = renderbuf +
					pixy * CHUNK_WIDTH * TILE_SIZE * 4 +
					pixx * 4;
				memcpy(pix, imgptr + imgy * TILE_SIZE * 4, TILE_SIZE * 4);
			}
		}
	}

	visuals_->tex_.update(renderbuf, CHUNK_WIDTH * TILE_SIZE, CHUNK_HEIGHT * TILE_SIZE, 0, 0);
	visuals_->dirty_ = true;
}

void Chunk::draw(const Context &ctx, Win &win) {
	if (compressed_size_ != -1)
		decompress();

	if (need_render_) {
		render(ctx);
		need_render_ = false;
	}

	if (visuals_->dirty_) {
		visuals_->sprite_.setTexture(visuals_->tex_);
		visuals_->dirty_ = false;
	}

	win.setPos(pos_ * Vec2i(CHUNK_WIDTH, CHUNK_HEIGHT));
	win.draw(visuals_->sprite_);
}

}
