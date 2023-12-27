#include "Chunk.h"

#include <zlib.h>
#include <stdint.h>
#include <assert.h>

#include "log.h"

namespace Swan {

void Chunk::compress(Cygnet::Renderer &rnd)
{
	if (isCompressed()) {
		return;
	}

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
	}
	else if (ret == Z_BUF_ERROR) {
		info
			<< "Didn't compress chunk " << pos_ << " "
			<< "because compressing it would've made it bigger";
	}
	else {
		warn << "Chunk compression error: " << ret << " (Out of memory?)";
	}

	rnd.destroyChunk(renderChunk_);
	rnd.destroyChunkShadow(renderChunkShadow_);
	entities_.rehash(0);
}

void Chunk::decompress()
{
	if (!isCompressed()) {
		return;
	}

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

	info
		<< "Decompressed chunk " << pos_ << " from "
		<< compressedSize_ << " bytes to "
		<< DATA_SIZE << " bytes.";
	compressedSize_ = -1;

	needChunkRender_ = true;
}

void Chunk::draw(const Context &ctx, Cygnet::Renderer &rnd)
{
	if (isCompressed()) {
		return;
	}

	if (needChunkRender_) {
		renderChunk_ = rnd.createChunk(getTileData());
		renderChunkShadow_ = rnd.createChunkShadow(getLightData());
		needChunkRender_ = false;
		needLightRender_ = false;
	}
	else {
		for (auto &change: changeList_) {
			rnd.modifyChunk(renderChunk_, change.first, change.second);
		}
	}

	if (needLightRender_) {
		rnd.modifyChunkShadow(renderChunkShadow_, getLightData());
		needLightRender_ = false;
	}

	Vec2 pos = (Vec2)pos_ * Vec2{CHUNK_WIDTH, CHUNK_HEIGHT};
	rnd.drawChunk({pos, renderChunk_});
	rnd.drawChunkShadow({pos, renderChunkShadow_});
}

Chunk::TickAction Chunk::tick(float dt)
{
	assert(isActive());

	deactivateTimer_ -= dt;
	if (deactivateTimer_ <= 0 && entities_.size() == 0) {
		if (isModified_) {
			return TickAction::DEACTIVATE;
		}
		else {
			return TickAction::DELETE;
		}
	}

	return TickAction::NOTHING;
}

void Chunk::keepActive()
{
	deactivateTimer_ = DEACTIVATE_INTERVAL;
	decompress();
}

}
