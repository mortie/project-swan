#include "Game.h"

namespace Swan {

void Game::registerTile(std::string &name, Tile &tile) {
	Tile::TileID id = registered_tiles_.size();
	registered_tiles_.push_back(tile);
	tile_id_map_[name] = id;
}

Tile::TileID Game::getTileID(std::string &name) {
	return tile_id_map_[name];
}

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

}
