#include "WorldPlane.h"

#include <math.h>
#include <iostream>
#include <algorithm>

#include "log.h"
#include "World.h"
#include "Game.h"
#include "Clock.h"

namespace Swan {

static ChunkPos chunkPos(TilePos pos) {
	// This might look weird, but it reduces an otherwise complex series of operations
	// including conditional branches into like four x64 instructions.
	// Basically, the problem is that we want 'floor(pos.x / CHUNK_WIDTH)', but
	// integer division rounds towards zero, it doesn't round down.
	// Solution: Move the position far to the right in the number line, do the math there
	// with integer division which always rounds down (because the numbers are always >0),
	// then move the result back to something hovering around 0 again.
	return ChunkPos(
		((long long)pos.x + (LLONG_MAX / 2) + 1) / CHUNK_WIDTH - ((LLONG_MAX / 2) / CHUNK_WIDTH) - 1,
		((long long)pos.y + (LLONG_MAX / 2) + 1) / CHUNK_HEIGHT - ((LLONG_MAX / 2) / CHUNK_HEIGHT) - 1);
}

static Chunk::RelPos relPos(TilePos pos) {
	// This uses a similar trick to chunkPos to turn a mess of conditional moves
	// and math instructions into literally one movabs and one 'and'
	return Chunk::RelPos(
		(pos.x + (long long)CHUNK_WIDTH * ((LLONG_MAX / 2) / CHUNK_WIDTH)) % CHUNK_WIDTH,
		(pos.y + (long long)CHUNK_HEIGHT * ((LLONG_MAX / 2) / CHUNK_HEIGHT)) % CHUNK_HEIGHT);
}

Context WorldPlane::getContext() {
	return {
		.game = *world_->game_,
		.world = *world_,
		.plane = *this,
	};
}

WorldPlane::WorldPlane(
		ID id, World *world, std::unique_ptr<WorldGen> gen,
		std::vector<std::unique_ptr<EntityCollection>> &&colls):
			id_(id), world_(world), gen_(std::move(gen)),
			entColls_(std::move(colls)),
			lighting_(std::make_unique<LightServer>(*this)) {

	for (auto &coll: entColls_) {
		entCollsByType_[coll->type()] = coll.get();
		entCollsByName_[coll->name()] = coll.get();
	}
}

EntityRef WorldPlane::spawnEntity(const std::string &name, const Entity::PackObject &obj) {
	return entCollsByName_.at(name)->spawn(getContext(), obj);
}

bool WorldPlane::hasChunk(ChunkPos pos) {
	return chunks_.find(pos) != chunks_.end();
}

// This function will be a bit weird because it's a really fucking hot function.
Chunk &WorldPlane::getChunk(ChunkPos pos) {
	// First, look through all chunks which have been in use this tick
	for (auto [chpos, chunk]: tickChunks_) {
		if (chpos == pos)
			return *chunk;
	}

	Chunk &chunk = slowGetChunk(pos);
	tickChunks_.push_back({pos, &chunk});
	return chunk;
}

Chunk &WorldPlane::slowGetChunk(ChunkPos pos) {
	ZoneScopedN("WorldPlane slowGetChunk");
	auto iter = chunks_.find(pos);

	// Create chunk if that turns out to be necessary
	if (iter == chunks_.end()) {
		iter = chunks_.emplace(pos, Chunk(pos)).first;
		Chunk &chunk = iter->second;

		gen_->genChunk(*this, chunk);
		activeChunks_.push_back(&chunk);
		chunkInitList_.push_back(&chunk);

		// Need to tell the light engine too
		NewLightChunk lc;
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			for (int x = 0; x < CHUNK_WIDTH; ++x) {
				Tile::ID id = chunk.getTileID({ x, y });
				Tile &tile = world_->getTileByID(id);
				if (tile.isSolid) {
					lc.blocks[y * CHUNK_HEIGHT + x] = true;
				}
				if (tile.lightLevel > 0) {
					lc.lightSources[{x, y}] = tile.lightLevel;
				}
			}
		}

		lighting_->onChunkAdded(pos, std::move(lc));

	// Otherwise, it might not be active, so let's activate it
	} else if (!iter->second.isActive()) {
		iter->second.keepActive();
		activeChunks_.push_back(&iter->second);
		chunkInitList_.push_back(&iter->second);
	}

	return iter->second;
}

void WorldPlane::setTileID(TilePos pos, Tile::ID id) {
	Chunk &chunk = getChunk(chunkPos(pos));
	Chunk::RelPos rp = relPos(pos);

	Tile::ID old = chunk.getTileID(rp);
	if (id != old) {
		Tile &newTile = world_->getTileByID(id);
		Tile &oldTile = world_->getTileByID(old);
		chunk.setTileID(rp, id);

		if (!oldTile.isSolid && newTile.isSolid) {
			lighting_->onSolidBlockAdded(pos);
		} else if (oldTile.isSolid && !newTile.isSolid) {
			lighting_->onSolidBlockRemoved(pos);
		}

		if (newTile.lightLevel != oldTile.lightLevel) {
			if (oldTile.lightLevel > 0) {
				removeLight(pos, oldTile.lightLevel);
			}

			if (newTile.lightLevel > 0) {
				addLight(pos, newTile.lightLevel);
			}
		}
	}
}

void WorldPlane::setTile(TilePos pos, const std::string &name) {
	setTileID(pos, world_->getTileID(name));
}

Tile::ID WorldPlane::getTileID(TilePos pos) {
	return getChunk(chunkPos(pos)).getTileID(relPos(pos));
}

Tile &WorldPlane::getTile(TilePos pos) {
	return world_->getTileByID(getTileID(pos));
}

Iter<Entity *> WorldPlane::getEntsInArea(Vec2 center, float radius) {
	return Iter<Entity *>([] { return std::nullopt; });
	// TODO: this
	/*
	return mapFilter(entities_.begin(), entities_.end(), [=](std::unique_ptr<Entity> &ent)
			-> std::optional<Entity *> {

		// Filter out things which don't have bodies
		auto *hasBody = dynamic_cast<BodyTrait::HasBody *>(ent.get());
		if (hasBody == nullptr)
			return std::nullopt;

		// Filter out things which are too far away from 'center'
		auto &body = hasBody->getBody();
		auto bounds = body.getBounds();
		Vec2 entcenter = bounds.pos + (bounds.size / 2);
		auto dist = (entcenter - center).length();
		if (dist > radius)
			return std::nullopt;

		return ent.get();
	});
	*/
}

EntityRef WorldPlane::spawnPlayer() {
	return gen_->spawnPlayer(getContext());
}

void WorldPlane::breakTile(TilePos pos) {

	// If the block is already air, do nothing
	Tile::ID id = getTileID(pos);
	Tile::ID air = world_->getTileID("@::air");
	if (id == air)
		return;

	// Change tile to air and emit event
	setTileID(pos, air);
	world_->evtTileBreak_.emit(getContext(), pos, world_->getTileByID(id));
}

SDL_Color WorldPlane::backgroundColor() {
	return gen_->backgroundColor(world_->player_->pos);
}

void WorldPlane::draw(Cygnet::Renderer &rnd) {
	ZoneScopedN("WorldPlane draw");
	std::lock_guard<std::mutex> lock(mut_);
	auto ctx = getContext();
	auto &pbody = *(world_->player_);

	gen_->drawBackground(ctx, rnd, pbody.pos);

	ChunkPos pcpos = ChunkPos(
		(int)floor(pbody.pos.x / CHUNK_WIDTH),
		(int)floor(pbody.pos.y / CHUNK_HEIGHT));

	// Just init one chunk per frame
	if (chunkInitList_.size() > 0) {
		/*
		Chunk *chunk = chunkInitList_.front();
		chunkInitList_.pop_front();
		chunk->render(ctx, win.renderer_);
		TODO */
	}

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			auto iter = chunks_.find(pcpos + ChunkPos(x, y));
			if (iter != chunks_.end())
				iter->second.draw(ctx, rnd);
		}
	}

	for (auto &coll: entColls_)
		coll->draw(ctx, rnd);

	/*
	if (debugBoxes_.size() > 0) {
		for (auto &pos: debugBoxes_) {
			rnd.drawRect(pos, Vec2(1, 1));
		}
	}
	TODO */
}

void WorldPlane::update(float dt) {
	ZoneScopedN("WorldPlane update");
	std::lock_guard<std::mutex> lock(mut_);
	auto ctx = getContext();
	debugBoxes_.clear();

	for (auto &coll: entColls_)
		coll->update(ctx, dt);
}

void WorldPlane::tick(float dt) {
	ZoneScopedN("WorldPlane tick");
	std::lock_guard<std::mutex> lock(mut_);
	auto ctx = getContext();

	// Any chunk which has been in use since last tick should be kept alive
	for (std::pair<ChunkPos, Chunk *> &ch: tickChunks_)
		ch.second->keepActive();
	tickChunks_.clear();

	for (auto &coll: entColls_)
		coll->tick(ctx, dt);

	// Tick all chunks, figure out if any of them should be deleted or compressed
	auto iter = activeChunks_.begin();
	auto last = activeChunks_.end();
	while (iter != last) {
		auto &chunk = *iter;
		auto action = chunk->tick(dt);

		switch (action) {
		case Chunk::TickAction::DEACTIVATE:
			info << "Compressing inactive modified chunk " << chunk->pos_;
			chunk->compress(world_->game_->renderer_);
			iter = activeChunks_.erase(iter);
			last = activeChunks_.end();
			break;
		case Chunk::TickAction::DELETE:
			info << "Deleting inactive unmodified chunk " << chunk->pos_;
			chunk->destroy(world_->game_->renderer_);
			chunks_.erase(chunk->pos_);
			iter = activeChunks_.erase(iter);
			last = activeChunks_.end();
			break;
		case Chunk::TickAction::NOTHING:
			++iter;
			break;
		}
	}
}

void WorldPlane::debugBox(TilePos pos) {
	debugBoxes_.push_back(pos);
}

void WorldPlane::addLight(TilePos pos, float level) {
	getChunk(chunkPos(pos));
	lighting_->onLightAdded(pos, level);
}

void WorldPlane::removeLight(TilePos pos, float level) {
	getChunk(chunkPos(pos));
	lighting_->onLightRemoved(pos, level);
}

void WorldPlane::onLightChunkUpdated(const LightChunk &chunk, ChunkPos pos) {
	std::lock_guard<std::mutex> lock(mut_);
	Chunk &realChunk = getChunk(pos);
	realChunk.setLightData(chunk.lightLevels);
}

}
