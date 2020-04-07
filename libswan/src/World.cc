#include "World.h"

#include <algorithm>

#include "log.h"
#include "Game.h"
#include "Win.h"
#include "Clock.h"

namespace Swan {

static void chunkLine(int l, WorldPlane &plane, ChunkPos &abspos, const Vec2i &dir) {
	for (int i = 0; i < l; ++i) {
		plane.getChunk(abspos);
		abspos += dir;
	}
}

World::World(Game *game, unsigned long rand_seed):
		game_(game), random_(rand_seed), resources_(game->win_) {

	std::unique_ptr<Tile> invalid_tile = Tile::createInvalid(resources_);
	tiles_map_[invalid_tile->name_] = 0;

	// tiles_ is empty, so pushing back now will ensure invalid_tile
	// ends up at location 0
	tiles_.push_back(std::move(invalid_tile));

}

void World::ChunkRenderer::tick(WorldPlane &plane, ChunkPos abspos) {
	int l = 0;

	RTClock clock;
	for (int i = 0; i < 4; ++i) {
		chunkLine(l, plane, abspos, Vec2i(0, -1));
		chunkLine(l, plane, abspos, Vec2i(1, 0));
		l += 1;
		chunkLine(l, plane, abspos, Vec2i(0, 1));
		chunkLine(l, plane, abspos, Vec2i(-1, 0));
		l += 1;
	}
}

void World::addMod(std::unique_ptr<Mod> mod) {
	info << "World: adding mod " << mod->name_;

	for (auto i: mod->buildImages(game_->win_.renderer_)) {
		resources_.addImage(std::move(i));
	}

	for (auto t: mod->buildTiles(resources_)) {
		Tile::ID id = tiles_.size();
		tiles_map_[t->name_] = id;
		tiles_.push_back(std::move(t));
	}

	for (auto i: mod->buildItems(resources_)) {
		items_[i->name_] = std::move(i);
	}

	for (auto gen: mod->getWorldGens()) {
		worldgens_.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(gen.name),
			std::forward_as_tuple(gen));
	}

	for (auto ent: mod->getEntities()) {
		ents_.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(ent.name),
			std::forward_as_tuple(ent));
	}

	mods_.push_back(std::move(mod));
}

void World::setWorldGen(const std::string &gen) {
	default_world_gen_ = gen;
}

void World::spawnPlayer() {
	player_ = planes_[current_plane_].spawnPlayer();
}

void World::setCurrentPlane(WorldPlane &plane) {
	current_plane_ = plane.id_;
}

WorldPlane &World::addPlane(const std::string &gen) {
	WorldPlane::ID id = planes_.size();
	auto it = worldgens_.find(gen);
	if (it == end(worldgens_)) {
		panic << "Tried to add plane with non-existant world gen " << gen << "!";
		abort();
	}

	WorldGen::Factory &factory = it->second;
	std::unique_ptr<WorldGen> g = factory.create(*this);
	planes_.emplace_back(id, this, std::move(g));
	return planes_[id];
}

Item &World::getItem(const std::string &name) {
	auto iter = items_.find(name);
	if (iter == items_.end()) {
		warn << "Tried to get non-existant item " << name << "!";
		return *game_->invalid_item_;
	}

	return *iter->second;
}

Tile::ID World::getTileID(const std::string &name) {
	auto iter = tiles_map_.find(name);
	if (iter == tiles_map_.end()) {
		warn << "Tried to get non-existant item " << name << "!";
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

SDL_Color World::backgroundColor() {
	return planes_[current_plane_].backgroundColor();
}

void World::draw(Win &win) {
	auto bounds = player_->getBody().getBounds();
	win.cam_ = bounds.pos - (win.getSize() / 2) + (bounds.size / 2);
	planes_[current_plane_].draw(win);
}

void World::update(float dt) {
	for (auto &plane: planes_)
		plane.update(dt);
}

void World::tick(float dt) {
	for (auto &plane: planes_)
		plane.tick(dt);

	auto bounds = player_->getBody().getBounds();
	chunk_renderer_.tick(
		planes_[current_plane_],
		ChunkPos((int)bounds.pos.x / CHUNK_WIDTH, (int)bounds.pos.y / CHUNK_HEIGHT));
}

}
