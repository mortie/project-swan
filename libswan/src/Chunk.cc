#include "Chunk.h"

#include <cstdint>
#include <optional>
#include <vector>
#include <assert.h>
#include <bit>
#include <string.h>

#include <swan/log.h>
#include <swan/trace.h>
#include "Clock.h"
#include "World.h"
#include "Game.h"
#include "capnp/message.h"
#include "capnp/serialize-packed.h"
#include "kj/io.h"
#include "rle.h"
#include "swan.capnp.h"

namespace Swan {

static thread_local std::vector<uint8_t> scratchBuffer;

Chunk::Chunk(ChunkPos pos): pos_(pos)
{
	data_.reset(new uint8_t[DATA_SIZE]);
	memset(getLightData(), 0, LIGHT_DATA_SIZE);
	memset(getFluidData(), 0, FLUID_DATA_SIZE);

	Tile::ID *backgroundData = getBackgroundTileData();
	for (size_t i = 0; i < CHUNK_WIDTH * CHUNK_HEIGHT; ++i) {
		backgroundData[i] = World::AIR_TILE_ID;
	}
}

std::unique_ptr<uint8_t[]> Chunk::compressToBuffer(size_t &len) const
{
	if (isCompressed()) {
		return nullptr;
	}

	capnp::MallocMessageBuilder mb;
	auto root = mb.initRoot<proto::ChunkRLEData>();

	scratchBuffer.clear();
	rleEncode16SRO(scratchBuffer, {getTileData(), CHUNK_WIDTH * CHUNK_HEIGHT});
	auto tiles = root.initTiles(scratchBuffer.size());
	memcpy(&tiles.front(), scratchBuffer.data(), scratchBuffer.size());

	scratchBuffer.clear();
	rleEncode16SRO(scratchBuffer, {getBackgroundTileData(), CHUNK_WIDTH * CHUNK_HEIGHT});
	auto background = root.initBackground(scratchBuffer.size());
	memcpy(&background.front(), scratchBuffer.data(), scratchBuffer.size());

	scratchBuffer.clear();
	rleEncode8(scratchBuffer, {getFluidData(), FLUID_DATA_SIZE});
	auto fluid = root.initFluid(scratchBuffer.size());
	memcpy(&fluid.front(), scratchBuffer.data(), scratchBuffer.size());

	kj::VectorOutputStream out;
	capnp::writePackedMessage(out, mb);
	auto arr = out.getArray();

	auto data = std::make_unique<uint8_t[]>(arr.size());
	len = arr.size();
	memcpy(&data[0], &arr.front(), arr.size());

	return data;
}

void Chunk::compress()
{
	if (isCompressed()) {
		return;
	}

	size_t len;
	data_ = compressToBuffer(len);
	compressedSize_ = len;

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

	auto compressedData = std::move(data_);
	size_t compressedSize = compressedSize_;

	data_ = std::make_unique<uint8_t[]>(DATA_SIZE);
	compressedSize_ = -1;

	kj::ArrayInputStream stream(kj::ArrayPtr(compressedData.get(), compressedSize));
	capnp::PackedMessageReader reader(stream);
	auto root = reader.getRoot<proto::ChunkRLEData>();

	auto tiles = root.getTiles();
	rleDecode16SRO(
		{getTileData(), CHUNK_WIDTH * CHUNK_HEIGHT},
		{&tiles.front(), tiles.size()});
	auto background = root.getBackground();
	rleDecode16SRO(
		{getBackgroundTileData(), CHUNK_WIDTH * CHUNK_HEIGHT},
		{&background.front(), background.size()});
	auto fluid = root.getFluid();
	rleDecode8(
		{getFluidData(), FLUID_DATA_SIZE},
		{&fluid.front(), fluid.size()});
}

void Chunk::draw(Ctx &ctx, Cygnet::Renderer &rnd)
{
	if (isCompressed()) {
		return;
	}

	if (!isRendered_) {
		ZoneScopedN("Chunk render activate");
		renderChunk_ = rnd.createChunk(getTileData(), getBackgroundTileData());
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

	{
		ZoneScopedN("Chunk changes");
		int count = 0;
		while (!changeList_.empty() && count++ < 10) {
			auto &change = changeList_.front();
			rnd.modifyChunk(renderChunk_, change.first, change.second);
			changeList_.pop_front();
		}
	}

	{
		ZoneScopedN("Chunk background changes");
		int count = 0;
		while (!backgroundChangeList_.empty() && count++ < 10) {
			auto &change = backgroundChangeList_.front();
			rnd.modifyChunkBackground(renderChunk_, change.first, change.second);
			backgroundChangeList_.pop_front();
		}
	}

	if (needLightRender_) {
		ZoneScopedN("Chunk light render");
		rnd.modifyChunkShadow(renderChunkShadow_, getLightData());
		needLightRender_ = false;
	}

	if (isFluidModified_) {
		ZoneScopedN("Chunk fluid render");
		rnd.modifyChunkFluid(renderChunkFluid_, getFluidData());
		isFluidModified_ = false;
	}

	Vec2 pos = (Vec2)pos_ * Vec2{CHUNK_WIDTH, CHUNK_HEIGHT};
	rnd.drawChunk({pos, renderChunk_});
	rnd.drawChunkFluid({pos, renderChunkFluid_});

	if (!fluidMasks_.empty()) {
		ZoneScopedN("Chunk fluid masks");
		for (auto &[_, mask]: fluidMasks_) {
			rnd.drawFluidMask(mask);
		}
	}

	if (!ctx.game.debug_.disableShadows) {
		rnd.drawChunkShadow({pos, renderChunkShadow_});
	}
}

void Chunk::serialize(proto::Chunk::Builder w) const
{
	using Compression = proto::Chunk::Compression;
	std::unique_ptr<uint8_t[]> compressionBuf;

	uint8_t *dataPtr = nullptr;
	size_t dataLen = 0;
	Compression compression = Compression::NONE;

	// If the chunk is already compressed,
	// just re-use the buffer
	if (isCompressed()) {
		dataPtr = data_.get();
		dataLen = compressedSize_;
		compression = Compression::RLE;
	}

	// If not, we try to compress it.
	// If we don't gain anything by compressing,
	// just use it uncompressed.
	else {
		compressionBuf = compressToBuffer(dataLen);
		dataPtr = compressionBuf.get();
		compression = Compression::RLE;
	}

	auto pos = w.initPos();
	pos.setX(pos_.x);
	pos.setY(pos_.y);

	w.setCompression(compression);
	auto d = w.initData(dataLen);
	memcpy(&d.front(), dataPtr, dataLen);
}

void Chunk::deserialize(proto::Chunk::Reader r, std::span<Tile::ID> tileMap)
{
	isModified_ = true;
	pos_ = {r.getPos().getX(), r.getPos().getY()};

	bool wasCompressed = false;
	auto data = r.getData();
	switch (r.getCompression()) {
	case proto::Chunk::Compression::NONE:
		if (data.size() != PERSISTENT_DATA_SIZE) {
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
		warn << "Unsupported chunk compression: GZIP";
		decompress();
		deactivateTimer_ = DEACTIVATE_INTERVAL;
		break;

	case proto::Chunk::Compression::RLE:
		data_ = std::make_unique<uint8_t[]>(data.size());
		memcpy(data_.get(), &data.front(), data.size());
		compressedSize_ = data.size();
		deactivateTimer_ = DEACTIVATE_INTERVAL;

		decompress();
		wasCompressed = true;
		break;
	}

	// Fix up foreground tiles
	std::span<Tile::ID> tileData(getTileData(), CHUNK_WIDTH * CHUNK_HEIGHT);
	for (Tile::ID &tile: tileData) {
		if (tile > tileMap.size()) {
			tile = 0;
		}
		else {
			tile = tileMap[tile];
		}
	}

	// Fix up background tiles
	tileData = {getBackgroundTileData(), CHUNK_WIDTH * CHUNK_HEIGHT};
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
