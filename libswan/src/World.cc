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

	std::unique_ptr<Tile> invalidTile = Tile::createInvalid(resources_);
	tilesMap_[invalidTile->name_] = 0;

	// tiles_ is empty, so pushing back now will ensure invalid_tile
	// ends up at location 0
	tiles_.push_back(std::move(invalidTile));

	// We're also going to need an air tile at location 1
	tiles_.push_back(Tile::createAir(resources_));
	tilesMap_["@::air"] = 1;
}

void World::ChunkRenderer::tick(WorldPlane &plane, ChunkPos abspos) {
	ZoneScopedN("World::ChunkRenderer tick");
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
		tilesMap_[t->name_] = id;
		tiles_.push_back(std::move(t));
	}

	for (auto i: mod.buildItems(resources_)) {
		items_[i->name_] = std::move(i);
	}

	for (auto fact: mod.getWorldGens()) {
		worldgenFactories_.emplace(
			std::piecewise_construct,
			std::forward_as_tuple(fact.name),
			std::forward_as_tuple(fact));
	}

	for (auto fact: mod.getEntities()) {
		entCollFactories_.push_back(fact);
	}

	mods_.push_back(std::move(mod));
}

void World::setWorldGen(std::string gen) {
	defaultWorldGen_ = std::move(gen);
}

void World::spawnPlayer() {
	player_ = &((dynamic_cast<BodyTrait *>(
		planes_[currentPlane_]->spawnPlayer().get()))->get(BodyTrait::Tag{}));
}

void World::setCurrentPlane(WorldPlane &plane) {
	currentPlane_ = plane.id_;
}

WorldPlane &World::addPlane(const std::string &gen) {
	WorldPlane::ID id = planes_.size();
	auto it = worldgenFactories_.find(gen);
	if (it == worldgenFactories_.end()) {
		panic << "Tried to add plane with non-existant world gen " << gen << "!";
		abort();
	}

	std::vector<std::unique_ptr<EntityCollection>> colls;
	colls.reserve(entCollFactories_.size());
	for (auto &fact: entCollFactories_) {
		colls.emplace_back(fact.create(fact.name));
	}

	WorldGen::Factory &factory = it->second;
	std::unique_ptr<WorldGen> g = factory.create(*this);
	planes_.push_back(std::make_unique<WorldPlane>(
			id, this, std::move(g), std::move(colls)));
	return *planes_[id];
}

Item &World::getItem(const std::string &name) {
	auto iter = items_.find(name);
	if (iter == items_.end()) {
		warn << "Tried to get non-existant item " << name << "!";
		return *game_->invalidItem_;
	}

	return *iter->second;
}

Tile::ID World::getTileID(const std::string &name) {
	auto iter = tilesMap_.find(name);
	if (iter == tilesMap_.end()) {
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
	return planes_[currentPlane_]->backgroundColor();
}

void World::draw(Win &win) {
	ZoneScopedN("World draw");
	win.cam_ = player_->pos - (win.getSize() / 2) + (player_->size / 2);
	planes_[currentPlane_]->draw(win);
}

void World::update(float dt) {
	ZoneScopedN("World update");
	for (auto &plane: planes_)
		plane->update(dt);
}

void World::tick(float dt) {
	ZoneScopedN("World tick");
	for (auto &plane: planes_)
		plane->tick(dt);

	chunkRenderer_.tick(
		*planes_[currentPlane_],
		ChunkPos((int)player_->pos.x / CHUNK_WIDTH, (int)player_->pos.y / CHUNK_HEIGHT));
}

}
