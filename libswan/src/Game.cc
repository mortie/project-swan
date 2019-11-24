#include "Game.h"

#include <dlfcn.h>
#include <math.h>
#include <time.h>
#include <memory>

#include "Tile.h"
#include "Win.h"

namespace Swan {

void Game::createWorld(const std::string &worldgen) {
	world_.reset(new World(this, time(NULL)));

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
