#include "WorldPlane.h"

#include <math.h>
#include <iostream>
#include <algorithm>

#include "log.h"
#include "World.h"
#include "Game.h"
#include "Clock.h"

namespace Swan {

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
				if (tile.isOpaque) {
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
	Chunk &chunk = getChunk(tilePosToChunkPos(pos));
	ChunkRelPos rp = tilePosToChunkRelPos(pos);

	Tile::ID old = chunk.getTileID(rp);
	if (id != old) {
		Tile &newTile = world_->getTileByID(id);
		Tile &oldTile = world_->getTileByID(old);
		chunk.setTileID(rp, id);

		if (!oldTile.isOpaque && newTile.isOpaque) {
			lighting_->onSolidBlockAdded(pos);
		} else if (oldTile.isOpaque && !newTile.isOpaque) {
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

		if (newTile.onSpawn) {
			newTile.onSpawn(getContext(), pos);
		}
	}
}

void WorldPlane::setTile(TilePos pos, const std::string &name) {
	setTileID(pos, world_->getTileID(name));
}

Tile::ID WorldPlane::getTileID(TilePos pos) {
	return getChunk(tilePosToChunkPos(pos)).getTileID(tilePosToChunkRelPos(pos));
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

Cygnet::Color WorldPlane::backgroundColor() {
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

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			auto iter = chunks_.find(pcpos + ChunkPos(x, y));
			if (iter != chunks_.end()) {
				Chunk &chunk = iter->second;
				chunk.draw(ctx, rnd);

				if (ctx.game.debugDrawChunkBoundaries_) {
					Vec2i size = {CHUNK_WIDTH, CHUNK_HEIGHT};
					rnd.drawRect(chunk.pos_ * size, size, {0.7, 0.1, 0.2, 1}); 
				}
			}
		}
	}

	for (auto &coll: entColls_) {
		coll->draw(ctx, rnd);
	}

	lighting_->flip();

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

	// Just init one chunk per frame
	if (chunkInitList_.size() > 0) {
		Chunk *chunk = chunkInitList_.front();
		chunkInitList_.pop_front();

		TilePos base = chunk->pos_ * Vec2i{CHUNK_WIDTH, CHUNK_HEIGHT};
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			for (int x = 0; x < CHUNK_WIDTH; ++x) {
				Tile::ID id = chunk->getTileID({x, y});
				Tile &tile = world_->getTileByID(id);
				if (tile.onSpawn) {
					tile.onSpawn(getContext(), base + Vec2i{x, y});
				}
			}
		}

		gen_->initializeChunk(ctx, *chunk);
	}

	for (auto &coll: entColls_)
		coll->update(ctx, dt);

	for (auto &ref: entDespawnList_) {
		ref.coll_->erase(ctx, ref.id_);
	}
	entDespawnList_.clear();
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
		auto action = chunk->tick(ctx, dt);

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
	getChunk(tilePosToChunkPos(pos));
	lighting_->onLightAdded(pos, level);
}

void WorldPlane::removeLight(TilePos pos, float level) {
	getChunk(tilePosToChunkPos(pos));
	lighting_->onLightRemoved(pos, level);
}

void WorldPlane::onLightChunkUpdated(const LightChunk &chunk, ChunkPos pos) {
	std::lock_guard<std::mutex> lock(mut_);
	Chunk &realChunk = getChunk(pos);
	realChunk.setLightData(chunk.lightLevels);
}

}
