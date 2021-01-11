#include "Game.h"

#include <math.h>
#include <time.h>
#include <memory>

#include "log.h"
#include "Tile.h"
#include "OS.h"

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
		(int)floor(cam_.pos.x + mousePos.x / (Swan::TILE_SIZE * cam_.zoom)),
		(int)floor(cam_.pos.y + mousePos.y / (Swan::TILE_SIZE * cam_.zoom)));
}

SDL_Color Game::backgroundColor() {
	return world_->backgroundColor();
}

void Game::draw() {
	world_->draw(renderer_);
	renderer_.draw(cam_);
}

void Game::update(float dt) {
	// Zoom the window using the scroll wheel
	cam_.zoom += (float)wasWheelScrolled() * 0.1f * cam_.zoom;
	if (cam_.zoom > 3)
		cam_.zoom = 3;
	else if (cam_.zoom < 0.3)
		cam_.zoom = 0.3;

	world_->update(dt);

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
