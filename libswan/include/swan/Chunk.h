#pragma once

#include <span>
#include <string.h>
#include <stdint.h>
#include <memory>
#include <unordered_set>
#include <cygnet/Renderer.h>
#include <assert.h>
#include <msgstream/msgstream.h>

#include "common.h"
#include "Tile.h"
#include "EntityCollection.h"

namespace Swan {

class World;
class WorldPlane;
class Game;

class Chunk {
public:
	static constexpr size_t TILE_DATA_SIZE =
		CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID);
	static constexpr size_t LIGHT_DATA_SIZE =
		CHUNK_WIDTH * CHUNK_HEIGHT;

	static constexpr size_t DATA_SIZE =
		TILE_DATA_SIZE + LIGHT_DATA_SIZE;

	// What does this chunk want the world gen to do after a tick?
	enum class TickAction {
		DEACTIVATE,
		DELETE,
		NOTHING,
	};

	Chunk(ChunkPos pos): pos_(pos)
	{
		data_.reset(new uint8_t[DATA_SIZE]);
		memset(getLightData(), 0, CHUNK_WIDTH * CHUNK_HEIGHT);
	}

	Tile::ID *getTileData()
	{
		assert(isActive());
		return (Tile::ID *)data_.get();
	}

	const Tile::ID *getTileData() const
	{
		assert(isActive());
		return (Tile::ID *)data_.get();
	}

	uint8_t *getLightData()
	{
		assert(isActive());
		return data_.get() + CHUNK_WIDTH * CHUNK_HEIGHT * sizeof(Tile::ID);
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

	void serialize(MsgStream::Serializer &w);
	void deserialize(MsgStream::Parser &r, std::span<Tile::ID> tileMap);

	const ChunkPos pos_;
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

	// Only used by WorldPlane when initializing chunks
	bool wasModified_ = false;

	friend WorldPlane;
};

}
