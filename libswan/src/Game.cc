#include "Game.h"

#include <dlfcn.h>

namespace Swan {

void Game::loadMod(const std::string &path) {
	void *dl = dlopen(path.c_str(), RTLD_LAZY);
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
	mod_init(mod);
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
