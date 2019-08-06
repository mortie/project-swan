#include "WorldPlane.h"

#include <math.h>

#include "World.h"
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

Entity &WorldPlane::spawnEntity(const std::string &name, const Vec2 &pos) {
	if (world_->ents_.find(name) == world_->ents_.end()) {
		fprintf(stderr, "Tried to spawn non-existant entity %s!",
				name.c_str());
		abort();
	}

	Entity *ent = world_->ents_[name]->create(*world_, pos);
	entities_.push_back(std::unique_ptr<Entity>(ent));
	fprintf(stderr, "Spawned %s at %f,%f.\n", name.c_str(), pos.x_, pos.y_);
	return *ent;
}

bool WorldPlane::hasChunk(ChunkPos pos) {
	return chunks_.find(pos) != chunks_.end();
}

Chunk &WorldPlane::getChunk(ChunkPos pos) {
	auto iter = chunks_.find(pos);

	if (iter == chunks_.end()) {
		iter = chunks_.emplace(pos, new Chunk(pos)).first;
		gen_->genChunk(*this, *iter->second);
		iter->second->redraw(*world_);
	}

	return *iter->second;
}

void WorldPlane::setTileID(TilePos pos, Tile::ID id) {
	getChunk(chunkPos(pos)).setTileID(*world_, relPos(pos), id);
}

void WorldPlane::setTile(TilePos pos, const std::string &name) {
	setTileID(pos, world_->getTileID(name));
}

Tile &WorldPlane::getTile(TilePos pos) {
	return getChunk(chunkPos(pos)).getTile(*world_, relPos(pos));
}

Entity &WorldPlane::spawnPlayer() {
	return gen_->spawnPlayer(*this);
}

void WorldPlane::draw(Win &win) {
	const Vec2 &ppos = world_->player_->getPos();
	ChunkPos pcpos = ChunkPos(
		(int)floor(ppos.x_ / CHUNK_WIDTH),
		(int)floor(ppos.y_ / CHUNK_HEIGHT));

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			auto chunk = chunks_.find(pcpos + ChunkPos(x, y));
			if (chunk != chunks_.end())
				chunk->second->draw(win);
		}
	}

	//for (auto &ch: chunks_)
	//	ch.second->draw(win);
	for (auto &ent: entities_)
		ent->draw(win);

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

void WorldPlane::update(Game &game, float dt) {
	debug_boxes_.clear();
	for (auto &ent: entities_)
		ent->update(game, *this, dt);
}

void WorldPlane::tick() {
	for (auto &ent: entities_)
		ent->tick();
}

void WorldPlane::debugBox(TilePos pos) {
	debug_boxes_.push_back(pos);
}

}
