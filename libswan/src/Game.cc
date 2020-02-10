#include "Game.h"

#include <math.h>
#include <time.h>
#include <memory>

#include "log.h"
#include "Tile.h"
#include "OS.h"
#include "Win.h"

namespace Swan {

std::unique_ptr<Mod> Game::loadMod(const std::string &path) {
	OS::Dynlib dl(path + "/mod");
	auto init = dl.get<void (*)(Swan::Mod &)>("mod_init");
	if (init == NULL) {
		warn << path << ": No 'mod_init' function!";
		return nullptr;
	}

	std::unique_ptr<Mod> mod = std::make_unique<Mod>(path, std::move(dl));
	init(*mod);
	return mod;
}

void Game::createWorld(const std::string &worldgen, std::vector<std::unique_ptr<Mod>> &&mods) {
	world_.reset(new World(this, time(NULL)));

	for (auto &mod: mods) {
		world_->addMod(std::move(mod));
	}

	world_->setWorldGen(worldgen);
	world_->setCurrentPlane(world_->addPlane());
	world_->spawnPlayer();
}

TilePos Game::getMouseTile() {
	auto mousePos = getMousePos();
	return TilePos(
		(int)floor(win_.cam_.x + mousePos.x / (Swan::TILE_SIZE * win_.zoom_)),
		(int)floor(win_.cam_.y + mousePos.y / (Swan::TILE_SIZE * win_.zoom_)));
}

SDL_Color Game::backgroundColor() {
	return world_->backgroundColor();
}

void Game::draw() {
	world_->draw(win_);
}

void Game::update(float dt) {
	world_->update(dt);

	// Zoom the window using the scroll wheel
	win_.zoom_ += (float)wasWheelScrolled() * 0.1f * win_.zoom_;
	if (win_.zoom_ > 3)
		win_.zoom_ = 3;
	else if (win_.zoom_ < 0.3)
		win_.zoom_ = 0.3;

	did_scroll_ = 0;
	did_press_keys_.reset();
	did_release_keys_.reset();
	did_press_buttons_.reset();
	did_release_buttons_.reset();
}

void Game::tick(float dt) {
	world_->tick(dt);
}

}
