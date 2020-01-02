#include "WorldPlane.h"

#include <math.h>
#include <iostream>

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
	return { .game = *world_->game_, .world = *world_, .plane = *this, .resources = world_->resources_ };
}

Entity *WorldPlane::spawnEntity(const std::string &name, const SRF &params) {
	if (world_->ents_.find(name) == world_->ents_.end()) {
		panic << "Tried to spawn a non-existant entity " << name << "!";
		abort();
	}

	Entity *ent = world_->ents_[name]->create(getContext(), params);
	if (auto has_body = dynamic_cast<BodyTrait::HasBody *>(ent); has_body) {
		BodyTrait::Body &body = has_body->getBody();
		BodyTrait::Bounds bounds = body.getBounds();

		body.move({ 0.5f - bounds.size.x / 2, 0 });
	}

	spawn_list_.push_back(std::unique_ptr<Entity>(ent));
	info << "Spawned " << name << ". SRF: " << params;
	return ent;
}

void WorldPlane::despawnEntity(Entity &ent) {
	info << "Despawned entity.";
	despawn_list_.push_back(&ent);
}

bool WorldPlane::hasChunk(ChunkPos pos) {
	return chunks_.find(pos) != chunks_.end();
}

Chunk &WorldPlane::getChunk(ChunkPos pos) {
	auto iter = chunks_.find(pos);

	if (iter == chunks_.end()) {
		iter = chunks_.emplace(pos, Chunk(pos)).first;
		Chunk &chunk = iter->second;

		gen_->genChunk(*this, chunk);
		active_chunks_.insert(&chunk);
		chunk.render(getContext());
	} else if (iter->second.keepActive()) {
		active_chunks_.insert(&iter->second);
	}

	return iter->second;
}

void WorldPlane::setTileID(TilePos pos, Tile::ID id) {
	Chunk &chunk = getChunk(chunkPos(pos));
	Chunk::RelPos rp = relPos(pos);
	chunk.setTileID(rp, id);
	chunk.drawBlock(rp, world_->getTileByID(id));

	if (active_chunks_.find(&chunk) == active_chunks_.end())
		active_chunks_.insert(&chunk);
}

void WorldPlane::setTile(TilePos pos, const std::string &name) {
	setTileID(pos, world_->getTileID(name));
}

Tile::ID WorldPlane::getTileID(TilePos pos) {
	Chunk &chunk = getChunk(chunkPos(pos));

	if (active_chunks_.find(&chunk) == active_chunks_.end())
		active_chunks_.insert(&chunk);

	return chunk.getTileID(relPos(pos));

}

Tile &WorldPlane::getTile(TilePos pos) {
	return world_->getTileByID(getTileID(pos));
}

BodyTrait::HasBody *WorldPlane::spawnPlayer() {
	return gen_->spawnPlayer(*this);
}

void WorldPlane::breakBlock(TilePos pos) {

	// If the block is already air, do nothing
	Tile::ID id = getTileID(pos);
	Tile::ID air = world_->getTileID("core::air");
	if (id == air)
		return;

	// Change the block to air...
	setTileID(pos, air);

	// Then spawn an item stack entity.
	Tile &t = world_->getTileByID(id);
	if (t.dropped_item_ != std::nullopt) {
		spawnEntity("core::item-stack", SRFArray{
			new SRFFloatArray{ (float)pos.x, (float)pos.y },
			new SRFString{ *t.dropped_item_ },
		});
	}
}

void WorldPlane::draw(Win &win) {
	auto pbounds = world_->player_->getBody().getBounds();
	ChunkPos pcpos = ChunkPos(
		(int)floor(pbounds.pos.x / CHUNK_WIDTH),
		(int)floor(pbounds.pos.y / CHUNK_HEIGHT));

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			auto iter = chunks_.find(pcpos + ChunkPos(x, y));
			if (iter != chunks_.end())
				iter->second.draw(getContext(), win);
		}
	}

	for (auto &ent: entities_)
		ent->draw(getContext(), win);

	if (debug_boxes_.size() > 0) {
		for (auto &pos: debug_boxes_) {
			win.drawRect(pos, Vec2(1, 1));
		}
	}
}

void WorldPlane::update(float dt) {
	debug_boxes_.clear();

	for (auto &ent: entities_)
		ent->update(getContext(), dt);

	for (auto &ent: spawn_list_)
		entities_.push_back(std::move(ent));
	spawn_list_.clear();

	for (auto entptr: despawn_list_) {
		for (auto it = std::begin(entities_); it != std::end(entities_); ++it) {
			if (it->get() == entptr) {
				entities_.erase(it);
				break;
			}
		}
	}
	despawn_list_.clear();
}

void WorldPlane::tick(float dt) {
	for (auto &ent: entities_)
		ent->tick(getContext(), dt);

	for (auto &chunk: active_chunks_)
		chunk->tick(dt);

	std::erase_if(active_chunks_, [](Chunk *chunk) { return !chunk->isActive(); });
}

void WorldPlane::debugBox(TilePos pos) {
	debug_boxes_.push_back(pos);
}

}
