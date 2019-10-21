#include "WorldPlane.h"

#include <math.h>
#include <SFML/System/Clock.hpp>
#include <iostream>

#include "World.h"
#include "Game.h"
#include "Timer.h"

namespace Swan {

static ChunkPos chunkPos(TilePos pos) {
	int chx = pos.x_ / CHUNK_WIDTH;
	if (pos.x_ < 0 && pos.x_ % CHUNK_WIDTH != 0) chx -= 1;
	int chy = pos.y_ / CHUNK_HEIGHT;
	if (pos.y_ < 0 && pos.y_ % CHUNK_HEIGHT != 0) chy -= 1;
	return ChunkPos(chx, chy);
}

static Chunk::RelPos relPos(TilePos pos) {
	int rx = pos.x_ % CHUNK_WIDTH;
	if (rx < 0) rx += CHUNK_WIDTH;
	int ry = pos.y_ % CHUNK_HEIGHT;
	if (ry < 0) ry += CHUNK_HEIGHT;
	return Chunk::RelPos(rx, ry);
}

Context WorldPlane::getContext() {
	return { .game = *world_->game_, .world = *world_, .plane = *this };
}

Entity &WorldPlane::spawnEntity(const std::string &name, const SRF &params) {
	if (world_->ents_.find(name) == world_->ents_.end()) {
		fprintf(stderr, "Tried to spawn non-existant entity %s!", name.c_str());
		abort();
	}

	Entity *ent = world_->ents_[name]->create(getContext(), params);
	entities_.push_back(std::unique_ptr<Entity>(ent));
	fprintf(stderr, "Spawned %s. SRF: ", name.c_str());
	params.pretty(std::cerr) << '\n';
	return *ent;
}

bool WorldPlane::hasChunk(ChunkPos pos) {
	return chunks_.find(pos) != chunks_.end();
}

Chunk &WorldPlane::getChunk(ChunkPos pos) {
	auto iter = chunks_.find(pos);

	if (iter == chunks_.end()) {
		iter = chunks_.emplace(pos, Chunk(pos)).first;
		gen_->genChunk(*this, iter->second);
		active_chunks_.insert(&iter->second);
		iter->second.render(getContext());
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

Entity &WorldPlane::spawnPlayer() {
	return gen_->spawnPlayer(*this);
}

void WorldPlane::breakBlock(TilePos pos) {
	Tile &t = getTile(pos);
	setTile(pos, "core::air");

	if (t.dropped_item != "") {
		spawnEntity("core::item-stack", SRFArray{
			new SRFFloatArray{ (float)pos.x_, (float)pos.y_ },
			new SRFString{ t.dropped_item },
		});
	}
}

void WorldPlane::draw(Win &win) {
	auto pbounds = *world_->player_->getBounds();
	ChunkPos pcpos = ChunkPos(
		(int)floor(pbounds.pos.x_ / CHUNK_WIDTH),
		(int)floor(pbounds.pos.y_ / CHUNK_HEIGHT));

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
		sf::RectangleShape rect(Vec2(TILE_SIZE, TILE_SIZE));
		rect.setFillColor(sf::Color(60, 70, 200, 100));
		rect.setOutlineThickness(1);
		rect.setOutlineColor(sf::Color(50, 65, 170, 200));
		for (auto &pos: debug_boxes_) {
			win.setPos(pos);
			win.draw(rect);
		}
	}
}

void WorldPlane::update(float dt) {
	debug_boxes_.clear();

	// Don't use iterators, because an entity's update method might push_back to entities
	for (size_t len = entities_.size(), i = 0; i < len; ++i)
		entities_[i]->update(getContext(), dt);
}

void WorldPlane::tick() {
	for (auto &ent: entities_)
		ent->tick();

	for (auto &chunk: active_chunks_) {
		chunk->tick();
		if (!chunk->isActive())
			active_chunks_.erase(chunk);
	}
}

void WorldPlane::debugBox(TilePos pos) {
	debug_boxes_.push_back(pos);
}

}
