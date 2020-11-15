#include "WorldPlane.h"

#include <math.h>
#include <iostream>
#include <algorithm>

#include "log.h"
#include "World.h"
#include "Game.h"
#include "Clock.h"
#include "Win.h"

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
		.resources = world_->resources_
	};
}

WorldPlane::WorldPlane(
		ID id, World *world, std::unique_ptr<WorldGen> gen,
		std::vector<std::unique_ptr<EntityCollection>> &&colls):
			id_(id), world_(world), gen_(std::move(gen)),
			lighting_(std::make_unique<LightServer>(*this)),
			ent_colls_(std::move(colls)) {

	for (auto &coll: ent_colls_) {
		ent_colls_by_type_[coll->type()] = coll.get();
		ent_colls_by_name_[coll->name()] = coll.get();
	}
}

EntityRef WorldPlane::spawnEntity(const std::string &name, const Entity::PackObject &obj) {
	return ent_colls_by_name_.at(name)->spawn(getContext(), obj);
}

bool WorldPlane::hasChunk(ChunkPos pos) {
	return chunks_.find(pos) != chunks_.end();
}

// This function will be a bit weird because it's a really fucking hot function.
Chunk &WorldPlane::getChunk(ChunkPos pos) {
	// First, look through all chunks which have been in use this tick
	for (auto [chpos, chunk]: tick_chunks_) {
		if (chpos == pos)
			return *chunk;
	}

	Chunk &chunk = slowGetChunk(pos);
	tick_chunks_.push_back({ pos, &chunk });
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
		active_chunks_.push_back(&chunk);
		chunk_init_list_.push_back(&chunk);

	// Otherwise, it might not be active, so let's activate it
	} else if (!iter->second.isActive()) {
		iter->second.keepActive();
		active_chunks_.push_back(&iter->second);
		chunk_init_list_.push_back(&iter->second);
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
		chunk.setTileID(rp, id, newTile.image_.texture_.get());
		chunk.markModified();

		if (!oldTile.is_solid_ && newTile.is_solid_) {
			lighting_->onSolidBlockAdded(pos);
		} else if (oldTile.is_solid_ && !newTile.is_solid_) {
			lighting_->onSolidBlockRemoved(pos);
		}

		if (newTile.light_level_ != oldTile.light_level_) {
			if (oldTile.light_level_ > 0) {
				lighting_->onLightRemoved(pos, oldTile.light_level_);
				removeLight(pos, oldTile.light_level_);
			}

			if (newTile.light_level_ > 0) {
				lighting_->onLightAdded(pos, newTile.light_level_);
				addLight(pos, newTile.light_level_);
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
		auto *has_body = dynamic_cast<BodyTrait::HasBody *>(ent.get());
		if (has_body == nullptr)
			return std::nullopt;

		// Filter out things which are too far away from 'center'
		auto &body = has_body->getBody();
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
	world_->evt_tile_break_.emit(getContext(), pos, world_->getTileByID(id));
}

SDL_Color WorldPlane::backgroundColor() {
	return gen_->backgroundColor(world_->player_->pos);
}

void WorldPlane::draw(Win &win) {
	ZoneScopedN("WorldPlane draw");
	auto ctx = getContext();
	auto &pbody = *(world_->player_);

	gen_->drawBackground(ctx, win, pbody.pos);

	ChunkPos pcpos = ChunkPos(
		(int)floor(pbody.pos.x / CHUNK_WIDTH),
		(int)floor(pbody.pos.y / CHUNK_HEIGHT));

	// Just init one chunk per frame
	if (chunk_init_list_.size() > 0) {
		Chunk *chunk = chunk_init_list_.front();
		info << "render chunk " << chunk->pos_;
		chunk_init_list_.pop_front();
		chunk->render(ctx, win.renderer_);
	}

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			auto iter = chunks_.find(pcpos + ChunkPos(x, y));
			if (iter != chunks_.end())
				iter->second.draw(ctx, win);
		}
	}

	for (auto &coll: ent_colls_)
		coll->draw(ctx, win);

	if (debug_boxes_.size() > 0) {
		for (auto &pos: debug_boxes_) {
			win.drawRect(pos, Vec2(1, 1));
		}
	}
}

void WorldPlane::update(float dt) {
	ZoneScopedN("WorldPlane update");
	auto ctx = getContext();
	debug_boxes_.clear();

	for (auto &coll: ent_colls_)
		coll->update(ctx, dt);
}

void WorldPlane::tick(float dt) {
	ZoneScopedN("WorldPlane tick");
	auto ctx = getContext();

	// Any chunk which has been in use since last tick should be kept alive
	for (std::pair<ChunkPos, Chunk *> &ch: tick_chunks_)
		ch.second->keepActive();
	tick_chunks_.clear();

	for (auto &coll: ent_colls_)
		coll->tick(ctx, dt);

	// Tick all chunks, figure out if any of them should be deleted or compressed
	auto iter = active_chunks_.begin();
	auto last = active_chunks_.end();
	while (iter != last) {
		auto &chunk = *iter;
		auto action = chunk->tick(dt);

		switch (action) {
		case Chunk::TickAction::DEACTIVATE:
			info << "Compressing inactive modified chunk " << chunk->pos_;
			chunk->compress();
			iter = active_chunks_.erase(iter);
			last = active_chunks_.end();
			break;
		case Chunk::TickAction::DELETE:
			info << "Deleting inactive unmodified chunk " << chunk->pos_;
			chunks_.erase(chunk->pos_);
			iter = active_chunks_.erase(iter);
			last = active_chunks_.end();
			break;
		case Chunk::TickAction::NOTHING:
			++iter;
			break;
		}
	}
}

void WorldPlane::debugBox(TilePos pos) {
	debug_boxes_.push_back(pos);
}

void WorldPlane::addLight(TilePos pos, uint8_t level) {
	int sqrLevel = level * level;

	for (int y = -level; y <= level; ++y) {
		for (int x = -level; x <= level; ++x) {
			int sqrDist = x * x + y * y;
			if (sqrDist > sqrLevel) {
				continue;
			}

			int lightDiff = level - (int)sqrt(sqrDist);
			if (lightDiff <= 0) {
				continue;
			}

			TilePos tp = pos + Vec2i(x, y);
			ChunkPos cp = chunkPos(tp);
			Chunk::RelPos rp = relPos(tp);

			Chunk &ch = getChunk(cp);
			int light = (int)ch.getLightLevel(rp) + lightDiff;
			if (light > 255) {
				light = 255;
			}

			ch.setLightData(rp, light);
		}
	}
}

void WorldPlane::removeLight(TilePos pos, uint8_t level) {
	int sqrLevel = level * level;

	for (int y = -level; y <= level; ++y) {
		for (int x = -level; x <= level; ++x) {
			int sqrDist = x * x + y * y;
			if (sqrDist > sqrLevel) {
				continue;
			}

			int lightDiff = level - (int)sqrt(sqrDist);
			if (lightDiff <= 0) {
				continue;
			}

			TilePos tp = pos + Vec2i(x, y);
			ChunkPos cp = chunkPos(tp);
			Chunk::RelPos rp = relPos(tp);

			Chunk &ch = getChunk(cp);
			int light = (int)ch.getLightLevel(rp) - lightDiff;
			if (light < 0) {
				light = 0;
			}

			ch.setLightData(rp, light);
		}
	}
}

}
