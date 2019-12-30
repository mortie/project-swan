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
		return NULL;
	}

	std::unique_ptr<Mod> mod = std::make_unique<Mod>(path);
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

}
