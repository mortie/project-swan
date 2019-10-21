#include "Game.h"

#include <dlfcn.h>
#include <math.h>
#include <SFML/Window/Mouse.hpp>

#include "Tile.h"
#include "Asset.h"

namespace Swan {

void Game::loadMod(const std::string &path) {
	std::string dlpath = path + "/mod.so";
	void *dl = dlopen(dlpath.c_str(), RTLD_LAZY);
	if (dl == NULL) {
		fprintf(stderr, "%s\n", dlerror());
		return;
	}

	void (*mod_init)(Mod &) = (void (*)(Mod &))dlsym(dl, "mod_init");
	if (mod_init == NULL) {
		fprintf(stderr, "%s\n", dlerror());
	}

	registered_mods_.push_back(Mod());
	Mod &mod = registered_mods_.back();
	mod.path_ = path;
	mod_init(mod);
}

void Game::createWorld(const std::string &worldgen) {
	world_.reset(new World(this));
	for (auto &mod: registered_mods_) {
		world_->registerTile(std::shared_ptr<Tile>(Tile::createInvalid()));

		for (auto &tile: mod.tiles_)
			world_->registerTile(tile);
		for (auto &item: mod.items_)
			world_->registerItem(item);
		for (auto &worldgen: mod.worldgens_)
			world_->registerWorldGen(worldgen);
		for (auto &entity: mod.entities_)
			world_->registerEntity(entity);
		for (auto &asset: mod.assets_)
			world_->registerAsset(asset);
	}

	world_->setWorldGen(worldgen);
	world_->setCurrentPlane(world_->addPlane());
	world_->spawnPlayer();
}

TilePos Game::getMouseTile() {
	auto mousePos = getMousePos();
	return TilePos(
		(int)floor(win_.cam_.x + mousePos.x / (Swan::TILE_SIZE * win_.scale_)),
		(int)floor(win_.cam_.y + mousePos.y / (Swan::TILE_SIZE * win_.scale_)));
}

void Game::draw() {
	world_->draw(win_);
}

void Game::update(float dt) {
	world_->update(dt);
}

void Game::tick(float dt) {
	world_->tick(dt);
}

void Game::initGlobal() {
	Tile::initGlobal();
	Item::initGlobal();
	Asset::initGlobal();
}

}
