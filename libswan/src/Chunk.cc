#include "Chunk.h"

#include <zlib.h>
#include <stdint.h>
#include <assert.h>
#include <bit>
#include <string.h>

#include "log.h"
#include "World.h"
#include "Game.h"

namespace Swan {

void Chunk::compress()
{
	if (isCompressed()) {
		return;
	}

	// We only need a fixed-length temp buffer;
	// if the compressed data gets too big, there's no point in compressing
	uint8_t dest[TILE_DATA_SIZE + FLUID_DATA_SIZE];

	uLongf destlen = sizeof(dest);
	int ret = compress2(
		(Bytef *)dest, &destlen,
		(Bytef *)data_.get(), TILE_DATA_SIZE + FLUID_DATA_SIZE,
		Z_BEST_COMPRESSION);

	if (ret == Z_OK) {
		data_.reset(new uint8_t[destlen]);
		memcpy(data_.get(), dest, destlen);

		compressedSize_ = destlen;
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
		// Properly free entities array memory
		std::unordered_set<EntityRef> empty;
		entities_.swap(empty);
	}

	{
		// Properly free fluid masks memory
		std::vector<std::pair<ChunkRelPos, Cygnet::Renderer::DrawMask>> empty;
		fluidMasks_.swap(empty);
		std::unordered_map<ChunkRelPos, size_t> emptyMap;
		fluidMaskMap_.swap(emptyMap);
	}
}

void Chunk::decompress()
{
	if (!isCompressed()) {
		return;
	}

	auto dest = std::make_unique<uint8_t[]>(DATA_SIZE);
	uLongf destlen = TILE_DATA_SIZE + FLUID_DATA_SIZE;
	int ret = uncompress(
		dest.get(), &destlen,
		(Bytef *)data_.get(), compressedSize_);

	if (ret != Z_OK) {
		panic << "Decompressing chunk failed: " << ret;
		abort();
	}

	data_ = std::move(dest);
	compressedSize_ = -1;
}

void Chunk::draw(Ctx &ctx, Cygnet::Renderer &rnd)
{
	if (isCompressed()) {
		return;
	}

	if (!isRendered_) {
		renderChunk_ = rnd.createChunk(getTileData());
		renderChunkFluid_ = rnd.createChunkFluid(getFluidData());
		renderChunkShadow_ = rnd.createChunkShadow(getLightData());

		// Populate fluid masks
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			auto *row = getTileData() + (y * CHUNK_WIDTH);
			for (int x = 0; x < CHUNK_WIDTH; ++x) {
				Tile::ID id = row[x];
				auto &tile = ctx.world.getTileByID(id);
				auto mask = tile.more->fluidMask;
				if (!mask) {
					continue;
				}

				ChunkRelPos rp = {x, y};
				fluidMasks_.push_back({rp, Cygnet::Renderer::DrawMask {
					.pos = pos_.scale(CHUNK_WIDTH, CHUNK_HEIGHT) + rp,
					.mask = mask,
				}});
			}
		}
		fluidMaskMap_.reserve(fluidMasks_.size());
		for (size_t i = 0; i < fluidMasks_.size(); ++i) {
			fluidMaskMap_[fluidMasks_[i].first] = i;
		}

		isRendered_ = true;
		needLightRender_ = false;
		isFluidModified_ = false;
	}

	for (auto &change: changeList_) {
		rnd.modifyChunk(renderChunk_, change.first, change.second);
	}
	changeList_.clear();

	if (needLightRender_) {
		rnd.modifyChunkShadow(renderChunkShadow_, getLightData());
		needLightRender_ = false;
	}

	if (isFluidModified_) {
		rnd.modifyChunkFluid(renderChunkFluid_, getFluidData());
		isFluidModified_ = false;
	}

	Vec2 pos = (Vec2)pos_ * Vec2{CHUNK_WIDTH, CHUNK_HEIGHT};
	rnd.drawChunk({pos, renderChunk_});
	rnd.drawChunkFluid({pos, renderChunkFluid_});

	for (auto &[_, mask]: fluidMasks_) {
		rnd.drawFluidMask(mask);
	}

	if (!ctx.game.debug_.disableShadows) {
		rnd.drawChunkShadow({pos, renderChunkShadow_});
	}
}

void Chunk::serialize(proto::Chunk::Builder w)
{
	compress();

	auto pos = w.initPos();
	pos.setX(pos_.x);
	pos.setY(pos_.y);

	if (isCompressed()) {
		w.setCompression(proto::Chunk::Compression::GZIP);
		static_assert(std::endian::native == std::endian::little);
		auto data = w.initData(compressedSize_);
		memcpy(&data.front(), data_.get(), compressedSize_);
	}
	else {
		w.setCompression(proto::Chunk::Compression::NONE);
		static_assert(std::endian::native == std::endian::little);
		auto data = w.initData(TILE_DATA_SIZE);
		memcpy(&data.front(), getTileData(), TILE_DATA_SIZE);
	}
}

void Chunk::deserialize(proto::Chunk::Reader r, std::span<Tile::ID> tileMap)
{
	isModified_ = true;
	pos_ = {r.getPos().getX(), r.getPos().getY()};

	bool wasCompressed = false;
	auto data = r.getData();
	switch (r.getCompression()) {
	case proto::Chunk::Compression::NONE:
		if (data.size() != TILE_DATA_SIZE) {
			throw std::runtime_error("Bad chunk size");
		}

		// We're probably already decompressed here,
		// but in case we're not, decompress
		decompress();

		static_assert(std::endian::native == std::endian::little);
		memcpy(data_.get(), &data.front(), data.size());
		deactivateTimer_ = DEACTIVATE_INTERVAL;
		break;

	case proto::Chunk::Compression::GZIP:
		data_ = std::make_unique<uint8_t[]>(data.size());
		static_assert(std::endian::native == std::endian::little);
		memcpy(data_.get(), &data.front(), data.size());
		compressedSize_ = data.size();
		deactivateTimer_ = DEACTIVATE_INTERVAL;

		decompress();
		wasCompressed = true;
		break;
	}

	std::span<Tile::ID> tileData(getTileData(), CHUNK_WIDTH * CHUNK_HEIGHT);
	for (Tile::ID &tile: tileData) {
		if (tile > tileMap.size()) {
			tile = 0;
		}
		else {
			tile = tileMap[tile];
		}
	}

	if (wasCompressed) {
		compress();
		deactivateTimer_ = 0;
	}
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

void Chunk::setFluidID(ChunkRelPos pos, Fluid::ID fluid)
{
	auto xStart = pos.x * FLUID_RESOLUTION;
	auto yStart = pos.y * FLUID_RESOLUTION;
	for (auto y = yStart; y < yStart + FLUID_RESOLUTION; ++y) {
		auto *row = getFluidData() + (y * CHUNK_WIDTH * FLUID_RESOLUTION);
		memset(row + xStart, fluid, FLUID_RESOLUTION);
	}
	isFluidModified_ = true;
}

void Chunk::setFluidSolid(ChunkRelPos pos, const FluidCollision &set)
{
	for (int y = 0; y < FLUID_RESOLUTION; ++y) {
		auto *row = getFluidData() +
			(pos.y * FLUID_RESOLUTION + y) * CHUNK_WIDTH * FLUID_RESOLUTION +
			pos.x * FLUID_RESOLUTION;
		for (int x = 0; x < FLUID_RESOLUTION; ++x) {
			if (set[y * FLUID_RESOLUTION + x]) {
				row[x] = World::SOLID_FLUID_ID;
			} else if (row[x] == World::SOLID_FLUID_ID) {
				row[x] = World::AIR_FLUID_ID;
			}
		}
	}
	isFluidModified_ = true;
}

void Chunk::clearFluidSolid(ChunkRelPos pos)
{
	for (int y = 0; y < FLUID_RESOLUTION; ++y) {
		auto *row = getFluidData() +
			(pos.y * FLUID_RESOLUTION + y) * CHUNK_WIDTH * FLUID_RESOLUTION +
			pos.x * FLUID_RESOLUTION;
		for (int x = 0; x < FLUID_RESOLUTION; ++x) {
			if (row[x] == World::SOLID_FLUID_ID) {
				row[x] = World::AIR_FLUID_ID;
			}
		}
	}
	isFluidModified_ = true;
}

void Chunk::setFluidMask(ChunkRelPos pos, Cygnet::RenderMask mask)
{
	Cygnet::Renderer::DrawMask dm = {
		.pos = pos_.scale(CHUNK_WIDTH, CHUNK_HEIGHT) + pos,
		.mask = mask,
	};

	auto it = fluidMaskMap_.find(pos);
	if (it == fluidMaskMap_.end()) {
		fluidMaskMap_[pos] = fluidMasks_.size();
		fluidMasks_.push_back({pos, dm});
	} else {
		auto idx = it->second;
		fluidMasks_[idx].second = dm;
	}
}

void Chunk::clearFluidMask(ChunkRelPos pos)
{
	auto it = fluidMaskMap_.find(pos);
	if (it == fluidMaskMap_.end()) {
		warn << "Attempt to clear fluid mask where there is none";
		return;
	}

	auto idx = it->second;
	if (idx != fluidMasks_.size() - 1) {
		auto back = fluidMasks_.back();
		fluidMasks_[idx] = fluidMasks_.back();
		fluidMaskMap_[back.first] = idx;
	}

	fluidMaskMap_.erase(pos);
	fluidMasks_.pop_back();
}

void Chunk::keepActive()
{
	deactivateTimer_ = DEACTIVATE_INTERVAL;
	decompress();
}

}
