#pragma once

#include <deque>
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

	static constexpr size_t BACKGROUND_TILE_DATA_SIZE = TILE_DATA_SIZE;
	static constexpr size_t BACKGROUND_TILE_DATA_OFFSET = TILE_DATA_OFFSET + TILE_DATA_SIZE;

	static constexpr size_t FLUID_DATA_SIZE =
		CHUNK_WIDTH * FLUID_RESOLUTION * CHUNK_HEIGHT * FLUID_RESOLUTION;
	static constexpr size_t FLUID_DATA_OFFSET = BACKGROUND_TILE_DATA_OFFSET + BACKGROUND_TILE_DATA_SIZE;

	static constexpr size_t LIGHT_DATA_SIZE =
		CHUNK_WIDTH * CHUNK_HEIGHT;
	static constexpr size_t LIGHT_DATA_OFFSET = FLUID_DATA_OFFSET + FLUID_DATA_SIZE;

	static constexpr size_t PERSISTENT_DATA_SIZE = TILE_DATA_SIZE + BACKGROUND_TILE_DATA_SIZE + FLUID_DATA_SIZE;
	static constexpr size_t DATA_SIZE = PERSISTENT_DATA_SIZE + LIGHT_DATA_SIZE + BACKGROUND_TILE_DATA_SIZE;

	// What does this chunk want the world gen to do after a tick?
	enum class TickAction {
		DEACTIVATE,
		DELETE,
		NOTHING,
	};

	Chunk(ChunkPos pos);

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

	Tile::ID *getBackgroundTileData()
	{
		assert(isActive());
		return (Tile::ID *)(data_.get() + BACKGROUND_TILE_DATA_OFFSET);
	}

	const Tile::ID *getBackgroundTileData() const
	{
		assert(isActive());
		return (Tile::ID *)(data_.get() + BACKGROUND_TILE_DATA_OFFSET);
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

	Tile::ID getBackgroundTileID(ChunkRelPos pos) const
	{
		return getBackgroundTileData()[pos.y * CHUNK_WIDTH + pos.x];
	}

	void setBackgroundTileID(ChunkRelPos pos, Tile::ID id)
	{
		getBackgroundTileData()[pos.y * CHUNK_WIDTH + pos.x] = id;
		backgroundChangeList_.emplace_back(pos, id);
		isModified_ = true;
	}

	void setFluidID(ChunkRelPos pos, Fluid::ID fluid);
	void setFluidSolid(ChunkRelPos pos, const FluidCollision &set);
	void clearFluidSolid(ChunkRelPos pos);

	void setFluidMask(ChunkRelPos pos, Cygnet::RenderMask mask);
	void clearFluidMask(ChunkRelPos pos);

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

	void draw(Ctx &ctx, Cygnet::Renderer &rnd);
	TickAction tick(float dt);

	bool isActive() const
	{
		return !isCompressed() && data_;
	}

	bool isModified() const
	{
		return isModified_;
	}

	ChunkPos pos() const
	{
		return pos_;
	}

	void setFluidModified()
	{
		isFluidModified_ = true;
	}

	size_t getMemUsage() const
	{
		return isCompressed() ? compressedSize_ : DATA_SIZE;
	}

	void serialize(proto::Chunk::Builder w) const;
	void deserialize(proto::Chunk::Reader r, std::span<Tile::ID> tileMap);

	std::unordered_set<EntityRef> entities_;
	uint64_t lightGeneration_ = 0;

private:
	static constexpr float DEACTIVATE_INTERVAL = 20;

	bool isCompressed() const
	{
		return compressedSize_ != -1;
	}

	std::unique_ptr<uint8_t[]> data_;
	std::deque<std::pair<ChunkRelPos, Tile::ID>> changeList_;
	std::deque<std::pair<ChunkRelPos, Tile::ID>> backgroundChangeList_;

	std::vector<std::pair<ChunkRelPos, Cygnet::Renderer::DrawMask>> fluidMasks_;
	std::unordered_map<ChunkRelPos, size_t> fluidMaskMap_;

	ssize_t compressedSize_ = -1; // -1 if not compressed, a positive number if compressed
	Cygnet::RenderChunk renderChunk_;
	Cygnet::RenderChunkFluid renderChunkFluid_;
	Cygnet::RenderChunkShadow renderChunkShadow_;
	bool needLightRender_ = false;
	float deactivateTimer_ = DEACTIVATE_INTERVAL;
	bool isModified_ = false;
	bool isFluidModified_ = false;
	bool isRendered_ = false;

	ChunkPos pos_;
};

}
