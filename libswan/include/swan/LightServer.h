#pragma once

#include <thread>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <bitset>

#include "common.h"

namespace Swan {

struct NewLightChunk {
	std::bitset<CHUNK_WIDTH * CHUNK_HEIGHT> blocks;
	std::map<std::pair<int, int>, int> light_sources;
};

struct LightChunk {
	LightChunk() = default;
	LightChunk(NewLightChunk &&ch):
		blocks(std::move(ch.blocks)), light_sources(std::move(ch.light_sources)) {}

	std::bitset<CHUNK_WIDTH * CHUNK_HEIGHT> blocks;
	uint8_t light_levels[CHUNK_WIDTH * CHUNK_HEIGHT] = { 0 };
	uint8_t blocks_line[CHUNK_WIDTH] = { 0 };
	std::map<std::pair<int, int>, int> light_sources;

	bool was_updated = false;
};

class LightCallback {
public:
	virtual void onLightChunkUpdated(const LightChunk &chunk, ChunkPos pos) = 0;
};

class LightServer {
public:
	LightServer(LightCallback &cb);
	~LightServer();

	void onSolidBlockAdded(TilePos pos);
	void onSolidBlockRemoved(TilePos pos);
	void onLightAdded(TilePos pos, int level);
	void onLightRemoved(TilePos pos, int level);
	void onChunkAdded(ChunkPos pos, NewLightChunk &&chunk);
	void onChunkRemoved(ChunkPos pos);

private:
	struct Event {
		enum class Tag {
			BLOCK_ADDED, BLOCK_REMOVED, LIGHT_ADDED, LIGHT_REMOVED,
			CHUNK_ADDED, CHUNK_REMOVED,
		} tag;

		TilePos pos;
		union {
			int num;
		};
	};

	bool tileIsSolid(TilePos pos);
	LightChunk *getChunk(ChunkPos cpos);

	int recalcTile(
			LightChunk &chunk, ChunkPos cpos, Vec2i rpos, TilePos base,
			std::vector<std::pair<TilePos, int>> &lights);
	void processUpdatedChunk(LightChunk &chunk, ChunkPos cpos);
	void processEvent(const Event &event, std::vector<NewLightChunk> &newChunks);
	void run();

	LightCallback &cb_;
	bool running_ = true;
	std::map<std::pair<int, int>, LightChunk> chunks_;
	std::set<std::pair<int, int>> updated_chunks_;
	LightChunk *cached_chunk_ = nullptr;
	Vec2i cached_chunk_pos_;

	int buffer_ = 0;
	std::vector<Event> buffers_[2] = { {}, {} };
	std::vector<NewLightChunk> new_chunk_buffers_[2] = { {}, {} };
	std::thread thread_;
	std::condition_variable cond_;
	std::mutex mut_;
};

inline void LightServer::onSolidBlockAdded(TilePos pos) {
	std::lock_guard<std::mutex> lock(mut_);
	buffers_[buffer_].push_back({ Event::Tag::BLOCK_ADDED, pos, { 0 } });
	cond_.notify_one();
}

inline void LightServer::onSolidBlockRemoved(TilePos pos) {
	std::lock_guard<std::mutex> lock(mut_);
	buffers_[buffer_].push_back({ Event::Tag::BLOCK_REMOVED, pos, { 0 } });
	cond_.notify_one();
}

inline void LightServer::onLightAdded(TilePos pos, int level) {
	std::lock_guard<std::mutex> lock(mut_);
	buffers_[buffer_].push_back({ Event::Tag::LIGHT_ADDED, pos, { .num = level } });
	cond_.notify_one();
}

inline void LightServer::onLightRemoved(TilePos pos, int level) {
	std::lock_guard<std::mutex> lock(mut_);
	buffers_[buffer_].push_back({ Event::Tag::LIGHT_REMOVED, pos, { .num = level  } });
	cond_.notify_one();
}

inline void LightServer::onChunkAdded(Vec2i pos, NewLightChunk &&chunk) {
	std::lock_guard<std::mutex> lock(mut_);
	buffers_[buffer_].push_back({ Event::Tag::CHUNK_ADDED, pos,
			{ .num = (int)new_chunk_buffers_[buffer_].size() } });
	new_chunk_buffers_[buffer_].push_back(std::move(chunk));
	cond_.notify_one();
}

inline void LightServer::onChunkRemoved(Vec2i pos) {
	std::lock_guard<std::mutex> lock(mut_);
	buffers_[buffer_].push_back({ Event::Tag::CHUNK_ADDED, pos, { 0 } });
	cond_.notify_one();
}

}
