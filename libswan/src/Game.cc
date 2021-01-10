#include "Game.h"

#include <math.h>
#include <time.h>
#include <memory>

#include "log.h"
#include "Tile.h"
#include "OS.h"
#include "Win.h"

namespace Swan {

void Game::createWorld(const std::string &worldgen, const std::vector<std::string> &modPaths) {
	world_.reset(new World(this, time(NULL), modPaths));

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

	didScroll_ = 0;
	didPressKeys_.reset();
	didReleaseKeys_.reset();
	didPressButtons_.reset();
	didReleaseButtons_.reset();
}

void Game::tick(float dt) {
	world_->tick(dt);
}

}
