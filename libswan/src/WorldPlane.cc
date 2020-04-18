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
	int chx = pos.x / CHUNK_WIDTH;
	if (pos.x < 0 && pos.x % CHUNK_WIDTH != 0) chx -= 1;
	int chy = pos.y / CHUNK_HEIGHT;
	if (pos.y < 0 && pos.y % CHUNK_HEIGHT != 0) chy -= 1;
	return ChunkPos(chx, chy);
}

static Chunk::RelPos relPos(TilePos pos) {
	int rx = pos.x % CHUNK_WIDTH;
	if (rx < 0) rx += CHUNK_WIDTH;
	int ry = pos.y % CHUNK_HEIGHT;
	if (ry < 0) ry += CHUNK_HEIGHT;
	return Chunk::RelPos(rx, ry);
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
			id_(id), world_(world), gen_(std::move(gen)), ent_colls_(std::move(colls)) {

	for (auto &coll: ent_colls_) {
		ent_colls_by_type_[coll->type()] = coll.get();
		ent_colls_by_name_[coll->name()] = coll.get();
	}
}

EntityRef WorldPlane::spawnEntity(const std::string &name, const Entity::PackObject &obj) {
	return ent_colls_by_name_.at(name)->spawn(getContext(), obj);
}

void WorldPlane::despawnEntity(Entity &ent) {
	// TODO: this
	info << "Despawned entity.";
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
		chunk.setTileID(rp, id, world_->getTileByID(id).image_.texture_.get());
		chunk.markModified();
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
	return gen_->backgroundColor(world_->player_->getBody().getBounds().pos);
}

void WorldPlane::draw(Win &win) {
	auto ctx = getContext();
	auto pbounds = world_->player_->getBody().getBounds();

	gen_->drawBackground(ctx, win, pbounds.pos);

	ChunkPos pcpos = ChunkPos(
		(int)floor(pbounds.pos.x / CHUNK_WIDTH),
		(int)floor(pbounds.pos.y / CHUNK_HEIGHT));

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
	auto ctx = getContext();
	debug_boxes_.clear();

	for (auto &coll: ent_colls_)
		coll->update(ctx, dt);
}

void WorldPlane::tick(float dt) {
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

}
