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

	// TODO: Delete renderChunk_
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

	// TODO: Create renderChunk_
}

void Chunk::draw(const Context &ctx, Cygnet::Renderer &rnd) {
	if (isCompressed())
		return;

	// The world plane is responsible for managing initial renders
	if (needRender_)
		return;

	if (drawList_.size() > 0) {
		//renderList(win.renderer_); TODO
		drawList_.clear();
	}

	// rnd.drawChunk(renderChunk_, (Vec2)pos_ * Vec2{CHUNK_WIDTH, CHUNK_HEIGHT}); TODO
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
