#pragma once

#include <span>
#include <string.h>
#include <stdint.h>
#include <memory>
#include <unordered_set>
#include <cygnet/Renderer.h>
#include <assert.h>

#include "common.h"
#include "Fluid.h"
#include "Tile.h"
#include "EntityCollection.h"
#include "swan.capnp.h"

namespace Swan {

class World;
class WorldPlane;
class Game;
class EntityCollection;

class Chunk {
public:
	static constexpr size_t TILE_DATA_SIZE =
		CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID);
	static constexpr size_t TILE_DATA_OFFSET = 0;

	static constexpr size_t FLUID_DATA_SIZE =
		CHUNK_WIDTH * FLUID_RESOLUTION * CHUNK_HEIGHT * FLUID_RESOLUTION;
	static constexpr size_t FLUID_DATA_OFFSET = TILE_DATA_OFFSET + TILE_DATA_SIZE;

	static constexpr size_t LIGHT_DATA_SIZE =
		CHUNK_WIDTH * CHUNK_HEIGHT;
	static constexpr size_t LIGHT_DATA_OFFSET = FLUID_DATA_OFFSET + FLUID_DATA_SIZE;

	static constexpr size_t DATA_SIZE =
		TILE_DATA_SIZE + FLUID_DATA_SIZE + LIGHT_DATA_SIZE;

	// What does this chunk want the world gen to do after a tick?
	enum class TickAction {
		DEACTIVATE,
		DELETE,
		NOTHING,
	};

	Chunk(ChunkPos pos): pos_(pos)
	{
		data_.reset(new uint8_t[DATA_SIZE]);
		memset(getLightData(), 0, LIGHT_DATA_SIZE);
		memset(getFluidData(), 0, FLUID_DATA_SIZE);
	}

	Tile::ID *getTileData()
	{
		assert(isActive());
		return (Tile::ID *)(data_.get() + TILE_DATA_OFFSET);
	}

	const Tile::ID *getTileData() const
	{
		assert(isActive());
		return (Tile::ID *)(data_.get() + TILE_DATA_OFFSET);
	}

	Fluid::ID *getFluidData()
	{
		assert(isActive());
		return (Fluid::ID *)(data_.get() + FLUID_DATA_OFFSET);
	}

	uint8_t *getLightData()
	{
		assert(isActive());
		return data_.get() + LIGHT_DATA_OFFSET;
	}

	Tile::ID getTileID(ChunkRelPos pos) const
	{
		return getTileData()[pos.y * CHUNK_WIDTH + pos.x];
	}

	void setTileID(ChunkRelPos pos, Tile::ID id)
	{
		getTileData()[pos.y * CHUNK_WIDTH + pos.x] = id;
		changeList_.emplace_back(pos, id);
		isModified_ = true;
	}

	void setTileData(ChunkRelPos pos, Tile::ID id)
	{
		getTileData()[pos.y * CHUNK_WIDTH + pos.x] = id;
	}

	void setFluidID(ChunkRelPos pos, Fluid::ID fluid)
	{
		auto xStart = pos.x * FLUID_RESOLUTION;
		auto yStart = pos.y * FLUID_RESOLUTION;
		for (auto y = yStart; y < yStart + FLUID_RESOLUTION; ++y) {
			auto *row = getFluidData() + (y * CHUNK_WIDTH * FLUID_RESOLUTION);
			memset(row + xStart, fluid, FLUID_RESOLUTION);
		}
	}

	uint8_t getLightLevel(ChunkRelPos pos)
	{
		return getLightData()[pos.y * CHUNK_WIDTH + pos.x];
	}

	void setLightData(const uint8_t *data)
	{
		memcpy(getLightData(), data, CHUNK_WIDTH * CHUNK_HEIGHT);
		needLightRender_ = true;
	}

	TilePos topLeft() const
	{
		return pos_ * TilePos{CHUNK_WIDTH, CHUNK_HEIGHT};
	}

	void generateDone();
	void keepActive();
	void decompress();
	void compress();

	void destroyTextures(Cygnet::Renderer &rnd)
	{
		if (isRendered_) {
			rnd.destroyChunk(renderChunk_);
			rnd.destroyChunkShadow(renderChunkShadow_);
			isRendered_ = false;
		}
	}

	void draw(const Context &ctx, Cygnet::Renderer &rnd);
	TickAction tick(float dt);

	bool isActive() const
	{
		return deactivateTimer_ > 0;
	}

	bool isModified() const
	{
		return isModified_;
	}

	ChunkPos pos() const
	{
		return pos_;
	}

	void serialize(proto::Chunk::Builder w);
	void deserialize(proto::Chunk::Reader r, std::span<Tile::ID> tileMap);

	std::unordered_set<EntityRef> entities_;

private:
	static constexpr float DEACTIVATE_INTERVAL = 20;

	bool isCompressed()
	{
		return compressedSize_ != -1;
	}

	std::unique_ptr<uint8_t[]> data_;
	std::vector<std::pair<ChunkRelPos, Tile::ID>> changeList_;

	ssize_t compressedSize_ = -1; // -1 if not compressed, a positive number if compressed
	Cygnet::RenderChunk renderChunk_;
	Cygnet::RenderChunkShadow renderChunkShadow_;
	bool needLightRender_ = false;
	float deactivateTimer_ = DEACTIVATE_INTERVAL;
	bool isModified_ = false;
	bool isRendered_ = false;

	ChunkPos pos_;
};

}
