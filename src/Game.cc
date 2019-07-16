#include "Game.h"

void Game::draw(Win &win) {
	for (WorldPlane *plane: planes_)
		plane->draw(win);

	player_->draw(win);
}

void Game::update(float dt) {
	for (WorldPlane *plane: planes_)
		plane->update(dt);

	player_->update(dt);
}

void Game::tick() {
	for (WorldPlane *plane: planes_)
		plane->tick();
}
