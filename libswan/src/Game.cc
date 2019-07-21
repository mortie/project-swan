#include "Game.h"

#include <dlfcn.h>

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

void Game::createWorld() {
	world_.reset(new World());
	for (auto &mod: registered_mods_) {
		for (auto &tile: mod.tiles_) {
			world_->registerTile(&tile);
		}
	}
}

void Game::draw(Win &win) {
	if (world_)
		world_->draw(win);
}

void Game::update(float dt) {
	if (world_)
		world_->update(dt);
}

void Game::tick() {
	if (world_)
		world_->tick();
}

}
