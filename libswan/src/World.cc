#include "World.h"

namespace Swan {

void World::ChunkRenderer::tick(WorldPlane &plane, ChunkPos abspos) {
	Vec2i dir(1, 0);
	ChunkPos relpos(-level_, -level_);

	if (!plane.hasChunk(abspos)) {
		plane.getChunk(abspos);
		level_ = 1;
		relpos = ChunkPos(-level_, -level_);
	}

	do {
		if (relpos == ChunkPos(level_, -level_))
			dir = Vec2i(0, 1);
		else if (relpos == ChunkPos(level_, level_))
			dir = Vec2i(-1, 0);
		else if (relpos == ChunkPos(-level_, level_))
			dir = Vec2i(0, -1);

		plane.getChunk(abspos + relpos);
		relpos += dir;
	} while (relpos != ChunkPos(-level_, -level_));

	if (level_ < 5)
		level_ += 1;
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
	auto size = win.window_->getSize();
	const Vec2 &pos = player_->getPos();
	win.cam_ = Vec2(
			pos.x_ * TILE_SIZE - size.x / 2,
			pos.y_ * TILE_SIZE - size.y / 2);

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
