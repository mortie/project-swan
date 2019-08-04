#include "Game.h"

#include <dlfcn.h>

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

void Game::createWorld(std::string worldgen) {
	world_.reset(new World());
	for (auto &mod: registered_mods_) {
		// Register invalids
		world_->registerTile(std::shared_ptr<Tile>(Tile::createInvalid()));

		for (auto &tile: mod.tiles_)
			world_->registerTile(tile);
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

void Game::draw(Win &win) {
	world_->draw(win);
}

void Game::update(float dt) {
	world_->update(dt);
}

void Game::tick() {
	world_->tick();
}

void Game::initGlobal() {
	Tile::initGlobal();
	Asset::initGlobal();
}

}
