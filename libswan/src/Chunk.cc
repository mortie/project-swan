#include "Chunk.h"

#include <zlib.h>
#include <stdint.h>
#include <assert.h>
#include <bit>
#include <string.h>

#include "log.h"

namespace Swan {

void Chunk::compress()
{
	if (isCompressed()) {
		return;
	}

	// We only need a fixed-length temp buffer;
	// if the compressed data gets too big, there's no point in compressing
	uint8_t dest[TILE_DATA_SIZE];

	uLongf destlen = sizeof(dest);
	int ret = compress2(
		(Bytef *)dest, &destlen,
		(Bytef *)data_.get(), TILE_DATA_SIZE,
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

	if (entities_.empty()) {
		std::unordered_set<EntityRef> empty;
		entities_.swap(empty);
	}
}

void Chunk::decompress()
{
	if (!isCompressed()) {
		return;
	}

	auto dest = std::make_unique<uint8_t[]>(DATA_SIZE);
	uLongf destlen = TILE_DATA_SIZE;
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
}

void Chunk::draw(const Context &ctx, Cygnet::Renderer &rnd)
{
	if (isCompressed()) {
		return;
	}

	if (!isRendered_) {
		renderChunk_ = rnd.createChunk(getTileData());
		renderChunkShadow_ = rnd.createChunkShadow(getLightData());
		isRendered_ = true;
		needLightRender_ = false;
	}

	for (auto &change: changeList_) {
		rnd.modifyChunk(renderChunk_, change.first, change.second);
	}
	changeList_.clear();

	if (needLightRender_) {
		rnd.modifyChunkShadow(renderChunkShadow_, getLightData());
		needLightRender_ = false;
	}

	Vec2 pos = (Vec2)pos_ * Vec2{CHUNK_WIDTH, CHUNK_HEIGHT};
	rnd.drawChunk({pos, renderChunk_});
	rnd.drawChunkShadow({pos, renderChunkShadow_});
}

void Chunk::serialize(MsgStream::Serializer &w)
{
	auto arr = w.beginArray(2);

	if (isCompressed()) {
		arr.writeUInt(1);
		static_assert(std::endian::native == std::endian::little);
		arr.writeBinary({
			(unsigned char *)data_.get(),
			(size_t)compressedSize_});
	}
	else {
		arr.writeUInt(0);
		static_assert(std::endian::native == std::endian::little);
		arr.writeBinary({
			(unsigned char *)data_.get(),
			CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID)});
	}
	w.endArray(arr);
}

void Chunk::deserialize(MsgStream::Parser &p, std::span<Tile::ID> tileMap)
{
	isModified_ = true;

	bool wasCompressed = false;

	auto arr = p.nextArray();
	uint64_t compressionType = arr.nextUInt();
	if (compressionType == 0) {
		std::vector<unsigned char> vec;
		arr.nextBinary(vec);
		if (vec.size() != CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID)) {
			throw std::runtime_error("Bad chunk size");
		}

		// We're probably already decompressed here,
		// but in case we're not, decompress
		decompress();

		static_assert(std::endian::native == std::endian::little);
		memcpy(data_.get(), vec.data(), vec.size());
		deactivateTimer_ = DEACTIVATE_INTERVAL;
	}
	else if (compressionType == 1) {
		std::vector<unsigned char> vec;
		arr.nextBinary(vec);

		data_ = std::make_unique<uint8_t[]>(vec.size());
		static_assert(std::endian::native == std::endian::little);
		memcpy(data_.get(), vec.data(), vec.size());
		compressedSize_ = vec.size();
		deactivateTimer_ = DEACTIVATE_INTERVAL;

		decompress();
		wasCompressed = true;
	}
	else {
		throw std::runtime_error("Invalid compression type");
	}

	std::span<Tile::ID> tileData(getTileData(), CHUNK_WIDTH * CHUNK_HEIGHT);
	for (Tile::ID &tile: tileData) {
		if (tile > tileMap.size()) {
			tile = 0;
		} else {
			tile = tileMap[tile];
		}
	}

	if (wasCompressed) {
		compress();
		deactivateTimer_ = 0;
	}

	arr.skipAll();
}

Chunk::TickAction Chunk::tick(float dt)
{
	assert(isActive());

	deactivateTimer_ -= dt;
	if (deactivateTimer_ <= 0) {
		assert(entities_.size() == 0);
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
