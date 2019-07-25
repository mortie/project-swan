#include "WorldPlane.h"

#include "World.h"

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

	Entity *ent = world_->ents_[name]->create(pos);
	entities_.push_back(std::unique_ptr<Entity>(ent));
	fprintf(stderr, "Spawned %s at %f,%f.\n", name.c_str(), pos.x_, pos.y_);
	return *ent;
}

Chunk &WorldPlane::getChunk(ChunkPos pos) {
	auto iter = chunks_.find(pos);

	if (iter == chunks_.end()) {
		iter = chunks_.emplace(pos, new Chunk(pos)).first;
		gen_->genChunk(*this, *iter->second);
		iter->second->redraw(world_->tile_map_);
		fprintf(stderr, "Generated chunk %i,%i\n", pos.x_, pos.y_);
	}

	return *iter->second;
}

void WorldPlane::setTileID(TilePos pos, Tile::ID id) {
	getChunk(chunkPos(pos)).setTileID(world_->tile_map_, relPos(pos), id);
}

Tile &WorldPlane::getTile(TilePos pos) {
	return getChunk(chunkPos(pos)).getTile(world_->tile_map_, relPos(pos));
}

Entity &WorldPlane::spawnPlayer() {
	return gen_->spawnPlayer(*this);
}

void WorldPlane::draw(Win &win) {
	for (auto &ch: chunks_)
		ch.second->draw(win);
	for (auto &ent: entities_)
		ent->draw(win);
}

void WorldPlane::update(float dt) {
	for (auto &ent: entities_)
		ent->update(*this, dt);
}

void WorldPlane::tick() {
	for (auto &ent: entities_)
		ent->tick();
}

}
