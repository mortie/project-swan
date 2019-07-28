#include "World.h"

namespace Swan {

static bool chunkLine(int l, int &left, WorldPlane &plane, ChunkPos &abspos, const Vec2i &dir) {
	for (int i = 0; i < l; ++i) {
		if (!plane.hasChunk(abspos)) {
			plane.getChunk(abspos);
			if (--left == 0)
				return true;
		}
		abspos += dir;
	}

	return false;
}

void World::ChunkRenderer::tick(WorldPlane &plane, ChunkPos abspos) {
	int l = 0;
	int left = 4;

	for (int i = 0; i < 8; ++i) {
		if (chunkLine(l, left, plane, abspos, Vec2i(0, -1))) return;
		if (chunkLine(l, left, plane, abspos, Vec2i(1, 0))) return;
		l += 1;
		if (chunkLine(l, left, plane, abspos, Vec2i(0, 1))) return;
		if (chunkLine(l, left, plane, abspos, Vec2i(-1, 0))) return;
		l += 1;
	}
}

void World::setCurrentPlane(WorldPlane &plane) {
	current_plane_ = plane.id_;
}

void World::setWorldGen(const std::string &gen) {
	default_world_gen_ = gen;
}

void World::spawnPlayer() {
	player_ = &planes_[current_plane_].spawnPlayer();
}

void World::registerTile(std::shared_ptr<Tile> t) {
	tile_map_.registerTile(t);
}

void World::registerWorldGen(std::shared_ptr<WorldGen::Factory> gen) {
	worldgens_[gen->name_] = gen;
}

void World::registerEntity(std::shared_ptr<Entity::Factory> ent) {
	ents_[ent->name_] = ent;
}

WorldPlane &World::addPlane(std::string gen) {
	WorldPlane::ID id = planes_.size();
	if (worldgens_.find(gen) == worldgens_.end()) {
		fprintf(stderr, "Tried to add plane with non-existant world gen '%s'!\n",
				gen.c_str());
		abort();
	}

	WorldGen *g = worldgens_[gen]->create(tile_map_);
	planes_.push_back(WorldPlane(id, this, std::shared_ptr<WorldGen>(g)));
	return planes_[id];
}

void World::draw(Win &win) {
	win.cam_ = player_->getPos() - (win.getSize() / 2) + 0.5;
	planes_[current_plane_].draw(win);
}

void World::update(float dt) {
	for (auto &plane: planes_)
		plane.update(dt);
}

void World::tick() {
	for (auto &plane: planes_)
		plane.tick();

	const Vec2 &abspos = player_->getPos();
	chunk_renderer_.tick(
			planes_[current_plane_],
			ChunkPos((int)abspos.x_ / CHUNK_WIDTH, (int)abspos.y_ / CHUNK_HEIGHT));
}

}
