#include "LightServer.h"

#include "log.h"

namespace Swan {

static ChunkPos lightChunkPos(TilePos pos) {
	// Same logic as in WorldPlane.cc
	return Vec2i(
		((size_t)pos.x + (LLONG_MAX / 2) + 1) / CHUNK_WIDTH -
			((LLONG_MAX / 2) / CHUNK_WIDTH) - 1,
		((size_t)pos.y + (LLONG_MAX / 2) + 1) / CHUNK_HEIGHT -
			((LLONG_MAX / 2) / CHUNK_HEIGHT) - 1);
}

static Vec2i lightRelPos(TilePos pos) {
	// Same logic as in WorldPlane.cc
	return Vec2i(
		(pos.x + (size_t)CHUNK_WIDTH * ((LLONG_MAX / 2) /
			CHUNK_WIDTH)) % CHUNK_WIDTH,
		(pos.y + (size_t)CHUNK_HEIGHT * ((LLONG_MAX / 2) /
			CHUNK_HEIGHT)) % CHUNK_HEIGHT);
}

LightServer::LightServer(LightCallback &cb):
	cb_(cb), thread_(&LightServer::run, this) {}

LightServer::~LightServer() {
	running_ = false;
	cond_.notify_one();
	thread_.join();
}

bool LightServer::tileIsSolid(TilePos pos) {
	ChunkPos cpos = lightChunkPos(pos);
	LightChunk *chunk = getChunk(cpos);
	if (chunk == nullptr) {
		return true;
	}

	Vec2i rpos = lightRelPos(pos);
	return chunk->blocks[rpos.y * CHUNK_WIDTH + rpos.x];
}

LightChunk *LightServer::getChunk(ChunkPos cpos) {
	if (cached_chunk_ && cached_chunk_pos_ == cpos) {
		return cached_chunk_;
	}

	auto it = chunks_.find(cpos);
	if (it != chunks_.end()) {
		cached_chunk_ = &it->second;
		cached_chunk_pos_ = cpos;
		return &it->second;
	}

	return nullptr;
}

void LightServer::processEvent(const Event &evt, std::vector<NewLightChunk> &newChunks) {
	// TODO: Only mark chunks within some sphere
	auto markChunksModified = [&](ChunkPos cpos) {
		for (int y = -1; y <= 1; ++y) {
			for (int x = -1; x <= 1; ++x) {
				updated_chunks_.insert(cpos + Vec2i(x, y));
			}
		}
	};

	if (evt.tag == Event::Tag::CHUNK_ADDED) {
		chunks_.emplace(std::piecewise_construct,
				std::forward_as_tuple(evt.pos),
				std::forward_as_tuple(std::move(newChunks[evt.num])));
		markChunksModified(evt.pos);
		return;
	} else if (evt.tag == Event::Tag::CHUNK_REMOVED) {
		chunks_.erase(evt.pos);
		markChunksModified(evt.pos);
		return;
	}

	ChunkPos cpos = lightChunkPos(evt.pos);
	LightChunk *ch = getChunk(cpos);
	if (!ch) return;
	markChunksModified(cpos);
	Vec2i rpos = lightRelPos(evt.pos);

	switch (evt.tag) {
	case Event::Tag::BLOCK_ADDED:
		ch->blocks.set(rpos.y * CHUNK_WIDTH + rpos.x, true);
		ch->blocks_line[rpos.x] += 1;
		break;

	case Event::Tag::BLOCK_REMOVED:
		ch->blocks.set(rpos.y * CHUNK_WIDTH + rpos.x, false);
		ch->blocks_line[rpos.x] -= 1;
		break;

	case Event::Tag::LIGHT_ADDED:
		info << cpos << ": Add " << evt.num << " light to " << rpos;
		ch->light_sources[rpos] += evt.num;
		break;

	case Event::Tag::LIGHT_REMOVED:
		info << cpos << ": Remove " << evt.num << " light to " << rpos;
		ch->light_sources[rpos] -= evt.num;
		break;

	// These were handled earlier
	case Event::Tag::CHUNK_ADDED:
	case Event::Tag::CHUNK_REMOVED:
		break;
	}
}

int LightServer::recalcTile(
		LightChunk &chunk, ChunkPos cpos, Vec2i rpos, TilePos base,
		std::vector<std::pair<TilePos, uint8_t>> &lights) {
	TilePos pos = rpos + base;

	constexpr int accuracy = 4;
	auto raycast = [&](Vec2 from, Vec2 to) {
		auto diff = to - from;
		float dist = ((Vec2)diff).length();
		Vec2 step = (Vec2)diff / (dist * accuracy);
		Vec2 currpos = from;
		TilePos currtile = TilePos(floor(currpos.x), floor(currpos.y));
		auto proceed = [&]() {
			TilePos t;
			while ((t = TilePos(floor(currpos.x), floor(currpos.y))) == currtile) {
				currpos += step;
			}
			currtile = t;
		};

		proceed();

		bool hit = false;
		while ((currpos - from).squareLength() <= diff.squareLength()) {
			if (tileIsSolid(currtile)) {
				hit = true;
				break;
			}

			proceed();
		}

		return hit;
	};

	int acc = 0;
	for (auto &[lightpos, level]: lights) {
		if (lightpos == pos) {
			acc += level;
			continue;
		}

		if ((lightpos - pos).squareLength() > level * level) {
			continue;
		}

		float dist = ((Vec2)(lightpos - pos)).length();
		int light = level - (int)dist;

		int hit =
			raycast(
				Vec2(pos.x + 0.3, pos.y + 0.3),
				Vec2(lightpos.x + 0.3, lightpos.y + 0.3)) +
			raycast(
				Vec2(pos.x + 0.7, pos.y + 0.3),
				Vec2(lightpos.x + 0.7, lightpos.y + 0.3)) +
			raycast(
				Vec2(pos.x + 0.3, pos.y + 0.7),
				Vec2(lightpos.x + 0.3, lightpos.y + 0.7)) +
			raycast(
				Vec2(pos.x + 0.7, pos.y + 0.7),
				Vec2(lightpos.x + 0.7, lightpos.y + 0.7));

		acc += (light * (4 - hit)) / 4;
		if (acc >= 255) {
			return 255;
		}
	}

	return acc;
}

void LightServer::processUpdatedChunk(LightChunk &chunk, ChunkPos cpos) {
	auto start = std::chrono::steady_clock::now();
	TilePos base = cpos * Vec2i(CHUNK_WIDTH, CHUNK_HEIGHT);
	std::vector<std::pair<TilePos, uint8_t>> lights;

	for (auto &[pos, level]: chunk.light_sources) {
		lights.emplace_back(Vec2i(pos) + base, level);
	}

	auto addLightFromChunk = [&](LightChunk *chunk, int dx, int dy) {
		if (chunk == nullptr) {
			return;
		}

		TilePos b = base + Vec2i(dx * CHUNK_WIDTH, dy * CHUNK_HEIGHT);
		for (auto &[pos, level]: chunk->light_sources) {
			lights.emplace_back(TilePos(pos) + b, level);
		}
	};

	for (int y = -1; y <= 1; ++y) {
		for (int x = -1; x <= 1; ++x) {
			if (y == 0 && x == 0) continue;
			addLightFromChunk(getChunk(cpos + Vec2i(x, y)), x, y);
		}
	}

	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			chunk.light_levels[y * CHUNK_WIDTH + x] =
				recalcTile(chunk, cpos, Vec2i(x, y), base, lights);
		}
	}

	auto end = std::chrono::steady_clock::now();
	auto dur = std::chrono::duration<double, std::milli>(end - start);
	info << "Generating light for " << cpos << " took " << dur.count() << "ms";

	cb_.onLightChunkUpdated(chunk, cpos);
}

void LightServer::run() {
	std::unique_lock<std::mutex> lock(mut_, std::defer_lock);
	while (running_) {
		lock.lock();
		cond_.wait(lock, [&] { return buffers_[buffer_].size() > 0 || !running_; });

		std::vector<Event> &buf = buffers_[buffer_];
		std::vector<NewLightChunk> &newChunks = new_chunk_buffers_[buffer_];
		buffer_ = (buffer_ + 1) % 2;
		lock.unlock();

		updated_chunks_.clear();
		for (auto &evt: buf) {
			processEvent(evt, newChunks);
		}

		buf.clear();
		newChunks.clear();

		for (auto &pos: updated_chunks_) {
			auto ch = chunks_.find(pos);
			if (ch != chunks_.end()) {
				processUpdatedChunk(ch->second, ChunkPos(pos.first, pos.second));
			}
		}

		updated_chunks_.clear();
	}
}

}
