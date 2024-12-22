#include "systems/LightSystem.h"

#include "WorldPlane.h"
#include "World.h"

namespace Swan {

void LightSystem::onLightChunkUpdated(const LightChunk &chunk, Vec2i pos)
{
	std::lock_guard<std::mutex> lock(mut_);
	updates_.push_back({});
	updates_.back().pos = pos;
	memcpy(updates_.back().levels, chunk.lightLevels, CHUNK_WIDTH * CHUNK_HEIGHT);
}

void LightSystem::addLight(TilePos pos, float level)
{
	plane_.getChunk(tilePosToChunkPos(pos));
	server_.onLightAdded(pos, level);
}

void LightSystem::removeLight(TilePos pos, float level)
{
	plane_.getChunk(tilePosToChunkPos(pos));
	server_.onLightRemoved(pos, level);
}

void LightSystem::addSolidBlock(TilePos pos)
{
	server_.onSolidBlockAdded(pos);
}

void LightSystem::removeSolidBlock(TilePos pos)
{
	server_.onSolidBlockRemoved(pos);
}

void LightSystem::addChunk(ChunkPos pos, const Chunk &chunk)
{
	server_.onChunkAdded(pos, computeLightChunk(chunk));
}

void LightSystem::removeChunk(ChunkPos pos)
{
	server_.onChunkRemoved(pos);
}

void LightSystem::flip()
{
	for (auto &update: updates_) {
		auto &chunk = plane_.getChunk(update.pos);
		chunk.setLightData(update.levels);
	}

	server_.flip();
}

NewLightChunk LightSystem::computeLightChunk(const Chunk &chunk)
{
	NewLightChunk lc;

	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			Tile::ID id = chunk.getTileID({x, y});
			Tile &tile = plane_.world_->getTileByID(id);
			if (tile.isOpaque) {
				lc.blocks[y * CHUNK_HEIGHT + x] = true;
			}
			if (tile.lightLevel > 0) {
				lc.lightSources[{x, y}] = tile.lightLevel;
			}
		}
	}

	return lc;
}

}
