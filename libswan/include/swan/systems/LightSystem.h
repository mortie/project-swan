#pragma once

#include "../LightServer.h"
#include "../common.h"

#include <cstdint>
#include <mutex>
#include <vector>

namespace Swan {

class WorldPlane;
class Chunk;

class LightSystem: public LightCallback {
public:
	LightSystem(WorldPlane &plane): plane_(plane) {}

	void addLight(TilePos pos, float level);
	void removeLight(TilePos pos, float level);

protected:
	// LightCallback implementation
	// TODO: Go away from a callback-based interface,
	// let the LightSystem ask the server for changed chunks instead
	void onLightChunkUpdated(const LightChunk &chunk, Vec2i pos) final;

private:
	struct LightUpdate {
		ChunkPos pos;
		uint8_t levels[CHUNK_WIDTH * CHUNK_HEIGHT];
	};

	void addSolidBlock(TilePos pos);
	void removeSolidBlock(TilePos pos);
	void addChunk(ChunkPos pos, const Chunk &chunk);
	void removeChunk(ChunkPos pos);
	void flip();

	NewLightChunk computeLightChunk(const Chunk &chunk);

	WorldPlane &plane_;
	LightServer server_{*this};
	std::vector<LightUpdate> updates_;

	std::mutex mut_;

	friend WorldPlane;
};

}
