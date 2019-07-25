#include "WorldPlane.h"

#include "World.h"

namespace Swan {

static Chunk::ChunkPos chunkPos(int x, int y) {
	int chx = x / CHUNK_WIDTH;
	if (x < 0 && x % CHUNK_WIDTH != 0) chx -= 1;
	int chy = y / CHUNK_HEIGHT;
	if (y < 0 && y % CHUNK_HEIGHT != 0) chy -= 1;
	return Chunk::ChunkPos(chx, chy);
}

static Chunk::RelPos relPos(int x, int y) {
	int rx = x % CHUNK_WIDTH;
	if (rx < 0) rx += CHUNK_WIDTH;
	int ry = y % CHUNK_HEIGHT;
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

Chunk &WorldPlane::getChunk(int x, int y) {
	Chunk::ChunkPos pos = chunkPos(x, y);
	auto iter = chunks_.find(pos);

	if (iter == chunks_.end()) {
		iter = chunks_.emplace(pos, new Chunk(pos)).first;
		gen_->genChunk(*this, *iter->second, pos.x_, pos.y_);
		iter->second->redraw(world_->tile_map_);
		fprintf(stderr, "Generated chunk %i,%i\n", pos.x_, pos.y_);
	}

	return *iter->second;
}

void WorldPlane::setTileID(int x, int y, Tile::ID id) {
	getChunk(x, y).setTileID(world_->tile_map_, relPos(x, y), id);
}

Tile &WorldPlane::getTile(int x, int y) {
	return getChunk(x, y).getTile(world_->tile_map_, relPos(x, y));
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
