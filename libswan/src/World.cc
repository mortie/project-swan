#include "World.h"

#include <algorithm>

#include "log.h"
#include "Game.h"
#include "Win.h"
#include "Clock.h"

namespace Swan {

static void chunkLine(int l, WorldPlane &plane, ChunkPos &abspos, const Vec2i &dir) {
	for (int i = 0; i < l; ++i) {
		plane.slowGetChunk(abspos).keepActive();
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

	// We're also going to need an air tile at location 1
	tiles_.push_back(Tile::createAir(resources_));
	tiles_map_["@::air"] = 1;

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

void World::addMod(ModWrapper &&mod) {
	info << "World: adding mod " << mod.mod_->name_;

	for (auto i: mod.buildImages(game_->win_.renderer_)) {
		resources_.addImage(std::move(i));
	}

	for (auto t: mod.buildTiles(resources_)) {
		Tile::ID id = tiles_.size();
		tiles_map_[t->name_] = id;
		tiles_.push_back(std::move(t));
	}

	for (auto i: mod.buildItems(resources_)) {
		items_[i->name_] = std::move(i);
	}

	for (auto fact: mod.getWorldGens()) {
		worldgen_factories_.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(fact.name),
			std::forward_as_tuple(fact));
	}

	for (auto fact: mod.getEntities()) {
		ent_coll_factories_.push_back(fact);
	}

	mods_.push_back(std::move(mod));
}

void World::setWorldGen(std::string gen) {
	default_world_gen_ = std::move(gen);
}

void World::spawnPlayer() {
	player_ = dynamic_cast<BodyTrait::HasBody *>(
		planes_[current_plane_].spawnPlayer().get());
}

void World::setCurrentPlane(WorldPlane &plane) {
	current_plane_ = plane.id_;
}

WorldPlane &World::addPlane(const std::string &gen) {
	WorldPlane::ID id = planes_.size();
	auto it = worldgen_factories_.find(gen);
	if (it == worldgen_factories_.end()) {
		panic << "Tried to add plane with non-existant world gen " << gen << "!";
		abort();
	}

	std::vector<std::unique_ptr<EntityCollection>> colls;
	colls.reserve(ent_coll_factories_.size());
	for (auto &fact: ent_coll_factories_) {
		colls.emplace_back(fact.create(fact.name));
	}

	WorldGen::Factory &factory = it->second;
	std::unique_ptr<WorldGen> g = factory.create(*this);
	planes_.emplace_back(id, this, std::move(g), std::move(colls));
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
