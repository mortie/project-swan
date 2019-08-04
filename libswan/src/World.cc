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
	Tile::ID id = tiles_.size();
	tiles_.push_back(t);
	tiles_map_[t->name_] = id;
}

void World::registerWorldGen(std::shared_ptr<WorldGen::Factory> gen) {
	worldgens_[gen->name_] = gen;
}

void World::registerEntity(std::shared_ptr<Entity::Factory> ent) {
	ents_[ent->name_] = ent;
}

void World::registerAsset(std::shared_ptr<Asset> asset) {
	assets_[asset->name_] = asset;
}

Asset &World::getAsset(const std::string &name) {
	auto iter = assets_.find(name);
	if (iter == assets_.end()) {
		fprintf(stderr, "Tried to get non-existant asset ''%s'!\n", name.c_str());
		return Asset::INVALID_ASSET;
	}

	return *iter->second;
}

Item &World::getItem(const std::string &name) {
	auto iter = items_.find(name);
	if (iter == items_.end()) {
		fprintf(stderr, "Tried to get non-existant item ''%s'!\n", name.c_str());
		return Item::INVALID_ITEM;
	}

	return *iter->second;
}

Tile::ID World::getTileID(const std::string &name) {
	auto iter = tiles_map_.find(name);
	if (iter == tiles_map_.end()) {
		fprintf(stderr, "Tried to get non-existant tile ''%s'!\n", name.c_str());
		return Tile::INVALID_ID;
	}

	return iter->second;
}

Tile &World::getTileByID(Tile::ID id) {
	return *tiles_[id];
}

Tile &World::getTile(const std::string &name) {
	Tile::ID id = getTileID(name);
	return getTileByID(id);
}

WorldPlane &World::addPlane(std::string gen) {
	WorldPlane::ID id = planes_.size();
	if (worldgens_.find(gen) == worldgens_.end()) {
		fprintf(stderr, "Tried to add plane with non-existant world gen '%s'!\n",
				gen.c_str());
		abort();
	}

	WorldGen *g = worldgens_[gen]->create(*this);
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
