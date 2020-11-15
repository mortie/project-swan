#include "LightingThread.h"

#include "log.h"

namespace Swan {

static Vec2i lightChunkPos(TilePos pos) {
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

LightingThread::LightingThread(LightingCallback &cb):
	cb_(cb), thread_(&LightingThread::run, this) {}

LightingThread::~LightingThread() {
	running_ = false;
	cond_.notify_one();
	thread_.join();
}

bool LightingThread::tileIsSolid(TilePos pos) {
	Vec2i cpos = lightChunkPos(pos);
	LightChunk *chunk = getChunk(cpos);
	if (chunk == nullptr) {
		return true;
	}

	Vec2i rpos = lightRelPos(pos);
	return chunk->blocks[rpos.y * CHUNK_WIDTH + rpos.x];
}

LightChunk *LightingThread::getChunk(Vec2i cpos) {
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

void LightingThread::processEvent(const Event &evt, std::vector<NewLightChunk> &newChunks) {
	info << "event " << (int)evt.tag;

	if (evt.tag == Event::Tag::CHUNK_ADDED) {
		chunks_.emplace(std::piecewise_construct,
				std::forward_as_tuple(evt.pos),
				std::forward_as_tuple(std::move(newChunks[evt.num])));
		LightChunk &ch = chunks_[evt.pos]; // Create and default initialize
		ch.was_updated = true;
		updated_chunks_.insert(evt.pos);
		return;
	} else if (evt.tag == Event::Tag::CHUNK_REMOVED) {
		chunks_.erase(evt.pos);
		return;
	}

	Vec2i cpos = lightChunkPos(evt.pos);
	LightChunk *ch = getChunk(cpos);
	if (!ch) return;
	ch->was_updated = true;
	updated_chunks_.insert(cpos);
	Vec2i rpos = lightRelPos(evt.pos);

	// TODO: Mark neighbouring chunks as updated

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
		ch->light_sources[rpos] += evt.num;
		break;

	case Event::Tag::LIGHT_REMOVED:
		ch->light_sources[rpos] -= evt.num;
		break;

	// These were handled earlier
	case Event::Tag::CHUNK_ADDED:
	case Event::Tag::CHUNK_REMOVED:
		break;
	}
}

int LightingThread::recalcTile(LightChunk &chunk, Vec2i cpos, Vec2i rpos, TilePos base) {
	std::vector<std::pair<Vec2i, uint8_t>> lights;
	TilePos pos = rpos + base;

	// TODO: Gather light sources from other chunks oo
	for (auto &[lightrel, level]: chunk.light_sources) {
		TilePos lightpos = base + Vec2i(lightrel.first, lightrel.second);
		Vec2i diff = lightpos - pos;
		if (diff.x * diff.x + diff.y * diff.y > level * level) {
			continue;
		}

		lights.push_back({ lightpos, level });
	}

	chunk.light_levels[rpos.y * CHUNK_WIDTH + rpos.x] = 0;

	constexpr int accuracy = 4;
	auto raycast = [&](Vec2 from, Vec2 to) {
		auto diff = to - from;
		float dist = ((Vec2)diff).length();
		Vec2 step = (Vec2)diff / (dist * accuracy);
		Vec2 currpos = from;
		Vec2i currtile = Vec2i(floor(currpos.x), floor(currpos.y));
		auto proceed = [&]() {
			Vec2i t;
			while ((t = Vec2i(floor(currpos.x), floor(currpos.y))) == currtile) {
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

void LightingThread::processUpdatedChunk(LightChunk &chunk, Vec2i cpos) {
	auto start = std::chrono::steady_clock::now();

	TilePos base = cpos * Vec2i(CHUNK_WIDTH * CHUNK_HEIGHT);
	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			chunk.light_levels[y * CHUNK_WIDTH + x] =
				recalcTile(chunk, cpos, Vec2i(x, y), base);
		}
	}

	auto end = std::chrono::steady_clock::now();
	auto dur = std::chrono::duration<double, std::milli>(end - start);
	info << "Generating light for " << cpos << " took " << dur.count() << "ms";

	cb_.onLightChunkUpdated(chunk, cpos);
}

void LightingThread::run() {
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
				processUpdatedChunk(ch->second, Vec2i(pos.first, pos.second));
			}
		}

		updated_chunks_.clear();
	}
}

}
