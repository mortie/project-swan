#include "LightServer.h"

#include <algorithm>

#include "log.h"

namespace Swan {

static ChunkPos lightChunkPos(TilePos pos)
{
	// Same logic as in WorldPlane.cc
	return Vec2i(
		((size_t)pos.x + (LLONG_MAX / 2) + 1) / CHUNK_WIDTH -
		((LLONG_MAX / 2) / CHUNK_WIDTH) - 1,
		((size_t)pos.y + (LLONG_MAX / 2) + 1) / CHUNK_HEIGHT -
		((LLONG_MAX / 2) / CHUNK_HEIGHT) - 1);
}

static Vec2i lightRelPos(TilePos pos)
{
	// Same logic as in WorldPlane.cc
	return Vec2i(
		(pos.x + (size_t)CHUNK_WIDTH * ((LLONG_MAX / 2) /
			CHUNK_WIDTH)) % CHUNK_WIDTH,
		(pos.y + (size_t)CHUNK_HEIGHT * ((LLONG_MAX / 2) /
			CHUNK_HEIGHT)) % CHUNK_HEIGHT);
}

static uint8_t linToSRGB(float lin)
{
	float s;

	if (lin <= 0.0031308) {
		s = lin / 12.92;
	}
	else {
		s = 1.055 * std::pow(lin, 1 / 2.4) - 0.055;
	}

	return std::clamp((int)round(s * 255), 0, 255);
}

static float attenuate(float dist, float squareDist)
{
	return 1 / (1 + 1 * dist + 0.02 * squareDist);
}

static float attenuate(float dist)
{
	return attenuate(dist, dist * dist);
}

static float attenuateSquared(float squareDist)
{
	return attenuate(sqrt(squareDist), squareDist);
}

LightChunk::LightChunk(NewLightChunk &&ch):
	blocks(std::move(ch.blocks)), lightSources(std::move(ch.lightSources))
{
	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			if (blocks[y * CHUNK_WIDTH + x]) {
				blocksLine[x] += 1;
			}
		}
	}
}

LightServer::LightServer(LightCallback &cb):
	cb_(cb), thread_(&LightServer::run, this)
{}

LightServer::~LightServer()
{
	running_ = false;
	cond_.notify_one();
	thread_.join();
}

bool LightServer::tileIsSolid(TilePos pos)
{
	ChunkPos cpos = lightChunkPos(pos);
	LightChunk *chunk = getChunk(cpos);

	if (chunk == nullptr) {
		return true;
	}

	Vec2i rpos = lightRelPos(pos);
	return chunk->blocks[rpos.y * CHUNK_WIDTH + rpos.x];
}

LightChunk *LightServer::getChunk(ChunkPos cpos)
{
	if (cachedChunk_ && cachedChunkPos_ == cpos) {
		return cachedChunk_;
	}

	auto it = chunks_.find(cpos);
	if (it != chunks_.end()) {
		cachedChunk_ = &it->second;
		cachedChunkPos_ = cpos;
		return &it->second;
	}

	return nullptr;
}

void LightServer::processEvent(const Event &evt, std::vector<NewLightChunk> &newChunks)
{
	auto markAdjacentChunksModified = [&](ChunkPos cpos) {
		for (int y = -1; y <= 1; ++y) {
			for (int x = -1; x <= 1; ++x) {
				updatedChunks_.insert(cpos + Vec2i(x, y));
			}
		}
	};

	auto markChunks = [&](ChunkPos cpos, Vec2i rpos, bool l, bool r, bool t, bool b) {
		updatedChunks_.insert(cpos);
		if (l) {
			updatedChunks_.insert(cpos + Vec2i(-1, 0));
		}
		if (r) {
			updatedChunks_.insert(cpos + Vec2i(1, 0));
		}
		if (t) {
			updatedChunks_.insert(cpos + Vec2i(0, -1));
		}
		if (b) {
			updatedChunks_.insert(cpos + Vec2i(0, 1));
		}
		if (l && t) {
			updatedChunks_.insert(cpos + Vec2i(-1, -1));
		}
		if (r && t) {
			updatedChunks_.insert(cpos + Vec2i(1, -1));
		}
		if (l && b) {
			updatedChunks_.insert(cpos + Vec2i(-1, 1));
		}
		if (r && b) {
			updatedChunks_.insert(cpos + Vec2i(1, 1));
		}
	};

	auto markChunksModified = [&](ChunkPos cpos, Vec2i rpos, float light) {
		markChunks(cpos, rpos,
			light * attenuate(std::max(rpos.x - 5, 0)) >= LIGHT_CUTOFF,
			light * attenuate(std::max(CHUNK_WIDTH - rpos.x - 5, 0)) >= LIGHT_CUTOFF,
			light * attenuate(std::max(rpos.y - 5, 0)) >= LIGHT_CUTOFF,
			light * attenuate(std::max(CHUNK_HEIGHT - rpos.y - 5, 0)) >= LIGHT_CUTOFF);
	};

	auto markChunksModifiedRange = [&](ChunkPos cpos, Vec2i rpos, int range) {
		markChunks(cpos, rpos,
			rpos.x <= range,
			CHUNK_WIDTH - rpos.x <= range,
			rpos.y <= range,
			CHUNK_WIDTH - rpos.y <= range);
	};

	if (evt.tag == Event::Tag::CHUNK_ADDED) {
		if (chunks_.contains(evt.pos)) {
			warn
				<< "LightServer: CHUNK_ADDED added an existing chunk "
				<< evt.pos;
			chunks_.erase(evt.pos);
		}

		chunks_.emplace(std::piecewise_construct,
			std::forward_as_tuple(evt.pos),
			std::forward_as_tuple(std::move(newChunks[evt.i])));
		markAdjacentChunksModified(evt.pos);
		return;
	}
	else if (evt.tag == Event::Tag::CHUNK_REMOVED) {
		if (!chunks_.contains(evt.pos)) {
			warn
				<< "LightServer: CHUNK_REMOVED removed a non-existent chunk "
				<< evt.pos;
			chunks_.erase(evt.pos);
		}

		chunks_.erase(evt.pos);
		markAdjacentChunksModified(evt.pos);
		return;
	}

	ChunkPos cpos = lightChunkPos(evt.pos);
	LightChunk *ch = getChunk(cpos);
	if (!ch) {
		return;
	}
	Vec2i rpos = lightRelPos(evt.pos);

	switch (evt.tag) {
	case Event::Tag::BLOCK_ADDED:
		ch->blocks.set(rpos.y * CHUNK_WIDTH + rpos.x, true);
		ch->blocksLine[rpos.x] += 1;
		markChunksModifiedRange(cpos, rpos, LIGHT_CUTOFF_DIST);
		break;

	case Event::Tag::BLOCK_REMOVED:
		ch->blocks.set(rpos.y * CHUNK_WIDTH + rpos.x, false);
		ch->blocksLine[rpos.x] -= 1;
		markChunksModifiedRange(cpos, rpos, LIGHT_CUTOFF_DIST);
		break;

	case Event::Tag::LIGHT_ADDED:
		ch->lightSources[rpos] += evt.f;
		markChunksModified(cpos, rpos, ch->lightSources[rpos]);
		break;

	case Event::Tag::LIGHT_REMOVED:
		markChunksModified(cpos, rpos, ch->lightSources[rpos]);
		ch->lightSources[rpos] -= evt.f;
		if (ch->lightSources[rpos] < LIGHT_CUTOFF) {
			ch->lightSources.erase(rpos);
		}
		break;

	// These were handled earlier
	case Event::Tag::CHUNK_ADDED:
	case Event::Tag::CHUNK_REMOVED:
		break;
	}
}

float LightServer::recalcTile(
	LightChunk &chunk, ChunkPos cpos, Vec2i rpos, TilePos base,
	std::vector<std::pair<TilePos, float>> &lights)
{
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

		bool hit = false;
		Vec2i target = TilePos(floor(to.x), floor(to.y));
		while (currtile != target && (currpos - from).squareLength() <= diff.squareLength()) {
			if (tileIsSolid(currtile)) {
				hit = true;
				break;
			}

			proceed();
		}

		return hit;
	};

	auto diffusedRaycast = [&](Vec2 from, Vec2 to, Vec2 norm) {
		if (raycast(from, to)) {
			return 0.0f;
		}

		float dot = (to - from).norm().dot(norm);
		if (dot > 1) {
			dot = 1;
		}
		else if (dot < 0) {
			dot = 0;
		}
		return dot;
	};

	bool isSolid = tileIsSolid(pos);

	bool culled =
		tileIsSolid(pos + Vec2i(-1, 0)) &&
		tileIsSolid(pos + Vec2i(1, 0)) &&
		tileIsSolid(pos + Vec2i(0, -1)) &&
		tileIsSolid(pos + Vec2i(0, 1));

	float acc = 0;

	for (auto &[lightpos, level]: lights) {
		if (lightpos == pos) {
			acc += level;
			continue;
		}

		if (culled) {
			continue;
		}

		int squareDist = (lightpos - pos).squareLength();
		if (squareDist > LIGHT_CUTOFF_DIST * LIGHT_CUTOFF_DIST) {
			continue;
		}

		float light = level * attenuateSquared(squareDist);
		if (light < LIGHT_CUTOFF) {
			continue;
		}

		if (!isSolid) {
			bool hit = raycast(
				Vec2(pos.x + 0.5, pos.y + 0.5),
				Vec2(lightpos.x + 0.5, lightpos.y + 0.5));
			if (!hit) {
				acc += light;
			}

			continue;
		}

		float frac = 0;
		float f;
		if ((f = diffusedRaycast(
			Vec2(pos.x + 0.5, pos.y - 0.1),
			Vec2(lightpos.x + 0.5, lightpos.y + 0.5),
			Vec2(0, -1))) > frac) {
			frac = f;
		}
		if ((f = diffusedRaycast(
			Vec2(pos.x + 0.5, pos.y + 1.1),
			Vec2(lightpos.x + 0.5, lightpos.y + 0.5),
			Vec2(0, 1))) > frac) {
			frac = f;
		}
		if ((f = diffusedRaycast(
			Vec2(pos.x - 0.1, pos.y + 0.5),
			Vec2(lightpos.x + 0.5, lightpos.y + 0.5),
			Vec2(-1, 0))) > frac) {
			frac = f;
		}
		if ((f = diffusedRaycast(
			Vec2(pos.x + 1.1, pos.y + 0.5),
			Vec2(lightpos.x + 0.5, lightpos.y + 0.5),
			Vec2(1, 0))) > frac) {
			frac = f;
		}

		acc += light * frac;
	}

	return acc;
}

void LightServer::processChunkSun(LightChunk &chunk, ChunkPos cpos)
{
	LightChunk *tc = getChunk(cpos + Vec2i(0, -1));

	int base = cpos.y * CHUNK_HEIGHT;

	std::bitset<CHUNK_WIDTH> line;

	for (int ry = 0; ry < CHUNK_HEIGHT; ++ry) {
		int y = base + ry;
		float light;
		if (y <= 20) {
			light = 1;
		}
		else {
			light = attenuate(y - 20);
			if (light < LIGHT_CUTOFF) {
				light = 0;
			}
		}

		for (int rx = 0; rx < CHUNK_WIDTH; ++rx) {
			bool lit = light > 0 && tc && tc->blocksLine[rx] == 0 && !line[rx];
			if (lit) {
				chunk.lightBuffer()[ry * CHUNK_WIDTH + rx] = light;
				if (chunk.blocks[ry * CHUNK_WIDTH + rx]) {
					line[rx] = true;
				}
			}
			else {
				chunk.lightBuffer()[ry * CHUNK_WIDTH + rx] = 0;
			}
		}
	}
}

void LightServer::processChunkLights(LightChunk &chunk, ChunkPos cpos)
{
	TilePos base = cpos * Vec2i(CHUNK_WIDTH, CHUNK_HEIGHT);
	std::vector<std::pair<TilePos, float>> lights;

	for (auto &[pos, level]: chunk.lightSources) {
		lights.emplace_back(Vec2i(pos) + base, level);
	}

	auto addLightFromChunk = [&](LightChunk *chunk, int dx, int dy) {
		if (chunk == nullptr) {
			return;
		}

		TilePos b = base + Vec2i(dx * CHUNK_WIDTH, dy * CHUNK_HEIGHT);
		for (auto &[pos, level]: chunk->lightSources) {
			lights.emplace_back(TilePos(pos) + b, level);
		}
	};

	for (int y = -1; y <= 1; ++y) {
		for (int x = -1; x <= 1; ++x) {
			if (y == 0 && x == 0) {
				continue;
			}
			addLightFromChunk(getChunk(cpos + Vec2i(x, y)), x, y);
		}
	}

	chunk.bounces.clear();
	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			float light = recalcTile(chunk, cpos, Vec2i(x, y), base, lights);
			chunk.lightBuffer()[y * CHUNK_WIDTH + x] += light;

			if (light > 0 && chunk.blocks[y * CHUNK_WIDTH + x]) {
				chunk.bounces.emplace_back(base + Vec2i(x, y), light * 0.1);
			}
		}
	}
}

void LightServer::processChunkBounces(LightChunk &chunk, ChunkPos cpos)
{
	TilePos base = cpos * Vec2i(CHUNK_WIDTH, CHUNK_HEIGHT);
	std::vector<std::pair<TilePos, float>> lights;

	for (auto &light: chunk.bounces) {
		lights.emplace_back(light);
	}

	auto addLightFromChunk = [&](LightChunk *chunk) {
		if (chunk == nullptr) {
			return;
		}

		for (auto &light: chunk->bounces) {
			lights.emplace_back(light);
		}
	};

	for (int y = -1; y <= 1; ++y) {
		for (int x = -1; x <= 1; ++x) {
			if (y == 0 && x == 0) {
				continue;
			}
			addLightFromChunk(getChunk(cpos + Vec2i(x, y)));
		}
	}

	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			float light = recalcTile(chunk, cpos, Vec2i(x, y), base, lights);
			float sum = chunk.lightBuffer()[y * CHUNK_WIDTH + x] + light;
			chunk.lightBuffer()[y * CHUNK_WIDTH + x] = sum;
		}
	}
}

void LightServer::processChunkSmoothing(LightChunk &chunk, ChunkPos cpos)
{
	LightChunk *tc = getChunk(cpos + Vec2i(0, -1));
	LightChunk *bc = getChunk(cpos + Vec2i(0, 1));
	LightChunk *lc = getChunk(cpos + Vec2i(-1, 0));
	LightChunk *rc = getChunk(cpos + Vec2i(1, 0));

	auto getLight = [&](LightChunk &chunk, int x, int y) {
		return chunk.lightBuffer()[y * CHUNK_WIDTH + x];
	};

	auto calc = [&](int x1, int x2, int y1, int y2, auto tf, auto bf, auto lf, auto rf) {
		float *dest = chunk.lightBuffers + CHUNK_WIDTH * CHUNK_HEIGHT * ((chunk.buffer + 1) % 2);
		for (int y = y1; y < y2; ++y) {
			for (int x = x1; x < x2; ++x) {
				float t = tf(x, y);
				float b = bf(x, y);
				float l = lf(x, y);
				float r = rf(x, y);
				float light = chunk.lightBuffer()[y * CHUNK_WIDTH + x];
				int count = 1;
				if (t > light) {
					light += t; count += 1;
				}
				if (b > light) {
					light += b; count += 1;
				}
				if (l > light) {
					light += l; count += 1;
				}
				if (r > light) {
					light += r; count += 1;
				}
				light /= count;
				dest[y * CHUNK_WIDTH + x] = light;
			}
		}
	};

	calc(1, CHUNK_WIDTH - 1, 1, CHUNK_HEIGHT - 1,
		[&](int x, int y) {
		return getLight(chunk, x, y - 1);
	},
		[&](int x, int y) {
		return getLight(chunk, x, y + 1);
	},
		[&](int x, int y) {
		return getLight(chunk, x - 1, y);
	},
		[&](int x, int y) {
		return getLight(chunk, x + 1, y);
	});

	if (tc) {
		calc(1, CHUNK_WIDTH - 1, 0, 1,
			[&](int x, int y) {
			return tc->lightBuffer()[(CHUNK_HEIGHT - 1) * CHUNK_WIDTH + x];
		},
			[&](int x, int y) {
			return getLight(chunk, x, y + 1);
		},
			[&](int x, int y) {
			return getLight(chunk, x - 1, y);
		},
			[&](int x, int y) {
			return getLight(chunk, x + 1, y);
		});
	}

	if (bc) {
		calc(1, CHUNK_WIDTH - 1, CHUNK_HEIGHT - 1, CHUNK_HEIGHT,
			[&](int x, int y) {
			return getLight(chunk, x, y - 1);
		},
			[&](int x, int y) {
			return bc->lightBuffer()[x];
		},
			[&](int x, int y) {
			return getLight(chunk, x - 1, y);
		},
			[&](int x, int y) {
			return getLight(chunk, x + 1, y);
		});
	}

	if (lc) {
		calc(0, 1, 1, CHUNK_HEIGHT - 1,
			[&](int x, int y) {
			return getLight(chunk, x, y - 1);
		},
			[&](int x, int y) {
			return getLight(chunk, x, y + 1);
		},
			[&](int x, int y) {
			return lc->lightBuffer()[y * CHUNK_WIDTH + CHUNK_WIDTH - 1];
		},
			[&](int x, int y) {
			return getLight(chunk, x + 1, y);
		});
	}

	if (rc) {
		calc(CHUNK_WIDTH - 1, CHUNK_WIDTH, 1, CHUNK_HEIGHT - 1,
			[&](int x, int y) {
			return getLight(chunk, x, y - 1);
		},
			[&](int x, int y) {
			return getLight(chunk, x, y + 1);
		},
			[&](int x, int y) {
			return getLight(chunk, x - 1, y);
		},
			[&](int x, int y) {
			return rc->lightBuffer()[y * CHUNK_WIDTH];
		});
	}

	if (tc && lc) {
		calc(0, 1, 0, 1,
			[&](int x, int y) {
			return tc->lightBuffer()[(CHUNK_HEIGHT - 1) * CHUNK_WIDTH + x];
		},
			[&](int x, int y) {
			return getLight(chunk, x, y + 1);
		},
			[&](int x, int y) {
			return lc->lightBuffer()[y * CHUNK_WIDTH + CHUNK_WIDTH - 1];
		},
			[&](int x, int y) {
			return getLight(chunk, x + 1, y);
		});
	}

	if (tc && rc) {
		calc(CHUNK_WIDTH - 1, CHUNK_WIDTH, 0, 1,
			[&](int x, int y) {
			return tc->lightBuffer()[(CHUNK_HEIGHT - 1) * CHUNK_WIDTH + x];
		},
			[&](int x, int y) {
			return getLight(chunk, x, y + 1);
		},
			[&](int x, int y) {
			return getLight(chunk, x - 1, y);
		},
			[&](int x, int y) {
			return rc->lightBuffer()[y * CHUNK_WIDTH];
		});
	}

	if (bc && lc) {
		calc(0, 1, CHUNK_HEIGHT - 1, CHUNK_HEIGHT,
			[&](int x, int y) {
			return getLight(chunk, x, y - 1);
		},
			[&](int x, int y) {
			return bc->lightBuffer()[x];
		},
			[&](int x, int y) {
			return lc->lightBuffer()[y * CHUNK_WIDTH + CHUNK_WIDTH - 1];
		},
			[&](int x, int y) {
			return getLight(chunk, x + 1, y);
		});
	}

	if (bc && rc) {
		calc(CHUNK_WIDTH - 1, CHUNK_WIDTH, CHUNK_HEIGHT - 1, CHUNK_HEIGHT,
			[&](int x, int y) {
			return getLight(chunk, x, y - 1);
		},
			[&](int x, int y) {
			return bc->lightBuffer()[x];
		},
			[&](int x, int y) {
			return getLight(chunk, x - 1, y);
		},
			[&](int x, int y) {
			return rc->lightBuffer()[y * CHUNK_WIDTH];
		});
	}
}

void LightServer::finalizeChunk(LightChunk &chunk)
{
	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			chunk.lightLevels[y * CHUNK_WIDTH + x] =
				linToSRGB(chunk.lightBuffer()[y * CHUNK_HEIGHT + x]);
		}
	}
}

void LightServer::run()
{
	std::unique_lock<std::mutex> lock(mut_, std::defer_lock);

	while (running_) {
		lock.lock();
		cond_.wait(lock, [&] {
			return buffers_[buffer_].size() > 0 || !running_;
		});

		std::vector<Event> &buf = buffers_[buffer_];
		std::vector<NewLightChunk> &newChunks = newChunkBuffers_[buffer_];
		buffer_ = (buffer_ + 1) % 2;
		lock.unlock();

		updatedChunks_.clear();
		for (auto &evt: buf) {
			processEvent(evt, newChunks);
		}

		buf.clear();
		newChunks.clear();

		for (auto &pos: updatedChunks_) {
			auto ch = chunks_.find(pos);
			if (ch != chunks_.end()) {
				processChunkSun(ch->second, pos);
			}
		}

		for (auto &pos: updatedChunks_) {
			auto ch = chunks_.find(pos);
			if (ch != chunks_.end()) {
				processChunkLights(ch->second, pos);
			}
		}

		for (auto &pos: updatedChunks_) {
			auto ch = chunks_.find(pos);
			if (ch != chunks_.end()) {
				processChunkBounces(ch->second, pos);
			}
		}

		for (int i = 0; i < 4; ++i) {
			for (auto &pos: updatedChunks_) {
				auto ch = chunks_.find(pos);
				if (ch != chunks_.end()) {
					processChunkSmoothing(ch->second, pos);
					ch->second.buffer = (ch->second.buffer + 1) % 2;
				}
			}
		}

		for (auto &pos: updatedChunks_) {
			auto ch = chunks_.find(pos);
			if (ch != chunks_.end()) {
				finalizeChunk(ch->second);
			}
		}

		for (auto &pos: updatedChunks_) {
			auto ch = chunks_.find(pos);
			if (ch != chunks_.end()) {
				cb_.onLightChunkUpdated(ch->second, pos);
			}
		}
	}
}

}
