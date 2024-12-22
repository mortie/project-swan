#pragma once

#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <bitset>
#include <unordered_map>
#include <unordered_set>

#include "common.h"

namespace Swan {

struct NewLightChunk {
	static constexpr size_t SIZE = CHUNK_WIDTH * CHUNK_HEIGHT;
	std::bitset<SIZE> blocks;
	std::unordered_map<ChunkPos, float> lightSources;
};

struct LightChunk {
	LightChunk() = default;
	LightChunk(NewLightChunk &&ch);

	std::bitset<CHUNK_WIDTH *CHUNK_HEIGHT> blocks;
	uint8_t lightLevels[CHUNK_WIDTH * CHUNK_HEIGHT] = {0};
	float lightBuffers[CHUNK_WIDTH * CHUNK_HEIGHT * 2] = {0};
	int buffer = 0;
	uint8_t blocksLine[CHUNK_WIDTH] = {0};
	std::unordered_map<ChunkPos, float> lightSources;
	std::vector<std::pair<TilePos, float>> bounces;

	float *lightBuffer()
	{
		return lightBuffers + CHUNK_WIDTH * CHUNK_HEIGHT * buffer;
	}

	bool wasUpdated = false;
};

class LightCallback {
public:
	virtual ~LightCallback() = default;

	virtual void onLightChunkUpdated(const LightChunk &chunk, ChunkPos pos) = 0;
};

class LightServer {
public:
	LightServer(LightCallback &cb);
	~LightServer();

	void onSolidBlockAdded(TilePos pos);
	void onSolidBlockRemoved(TilePos pos);
	void onLightAdded(TilePos pos, float level);
	void onLightRemoved(TilePos pos, float level);
	void onChunkAdded(ChunkPos pos, NewLightChunk &&chunk);
	void onChunkRemoved(ChunkPos pos);
	void flip();

	template<typename F>
	void getUpdates(F cb);

private:
	static constexpr int LIGHT_CUTOFF_DIST = 64;
	static constexpr float LIGHT_CUTOFF = 0.001;

	struct Event {
		enum class Tag {
			BLOCK_ADDED, BLOCK_REMOVED, LIGHT_ADDED, LIGHT_REMOVED,
			CHUNK_ADDED, CHUNK_REMOVED,
		} tag;

		TilePos pos;
		union {
			float f;
			int i;
		};
	};

	bool tileIsSolid(TilePos pos);
	LightChunk *getChunk(ChunkPos cpos);

	float recalcTile(
		LightChunk &chunk, ChunkPos cpos, Vec2i rpos, TilePos base,
		std::vector<std::pair<TilePos, float>> &lights);
	void processChunkSun(LightChunk &chunk, ChunkPos cpos);
	void processChunkLights(LightChunk &chunk, ChunkPos cpos);
	void processChunkBounces(LightChunk &chunk, ChunkPos cpos);
	void processChunkSmoothing(LightChunk &chunk, ChunkPos cpos);
	void finalizeChunk(LightChunk &chunk);
	void processEvent(const Event &event, std::vector<NewLightChunk> &newChunks);
	void run();

	bool running_ = true;
	std::unordered_map<ChunkPos, LightChunk> chunks_;
	std::unordered_set<ChunkPos> updatedChunks_;
	LightChunk *cachedChunk_ = nullptr;
	Vec2i cachedChunkPos_;

	int buffer_ = 0;
	std::vector<Event> buffers_[2] = {{}, {}};
	std::vector<NewLightChunk> newChunkBuffers_[2] = {{}, {}};
	std::condition_variable cond_;
	std::mutex mut_;

	LightCallback &cb_;
	std::thread thread_;
};

inline void LightServer::onSolidBlockAdded(TilePos pos)
{
	std::lock_guard<std::mutex> lock(mut_);

	buffers_[buffer_].push_back({Event::Tag::BLOCK_ADDED, pos, {.i = 0}});
}

inline void LightServer::onSolidBlockRemoved(TilePos pos)
{
	std::lock_guard<std::mutex> lock(mut_);

	buffers_[buffer_].push_back({Event::Tag::BLOCK_REMOVED, pos, {.i = 0}});
}

inline void LightServer::onLightAdded(TilePos pos, float level)
{
	std::lock_guard<std::mutex> lock(mut_);

	buffers_[buffer_].push_back({Event::Tag::LIGHT_ADDED, pos, {.f = level}});
}

inline void LightServer::onLightRemoved(TilePos pos, float level)
{
	std::lock_guard<std::mutex> lock(mut_);

	buffers_[buffer_].push_back({Event::Tag::LIGHT_REMOVED, pos, {.f = level}});
}

inline void LightServer::onChunkAdded(Vec2i pos, NewLightChunk &&chunk)
{
	std::lock_guard<std::mutex> lock(mut_);

	buffers_[buffer_].push_back({
		Event::Tag::CHUNK_ADDED, pos,
		{.i = (int)newChunkBuffers_[buffer_].size()}});
	newChunkBuffers_[buffer_].push_back(std::move(chunk));
}

inline void LightServer::onChunkRemoved(Vec2i pos)
{
	std::lock_guard<std::mutex> lock(mut_);

	buffers_[buffer_].push_back({Event::Tag::CHUNK_REMOVED, pos, {.i = 0}});
}

inline void LightServer::flip()
{
	cond_.notify_one();
}

}
