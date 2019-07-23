#include "World.h"

namespace Swan {

WorldPlane::ID World::addPlane(std::string gen) {
	WorldPlane::ID id = planes_.size();
	if (worldgens_.find(gen) == worldgens_.end()) {
		fprintf(stderr, "Tried to add plane with non-existant world gen '%s'!\n",
				gen.c_str());
		abort();
	}

	WorldGen *g = worldgens_[gen]->create(tile_map_);
	planes_.push_back(WorldPlane(id, this, std::shared_ptr<WorldGen>(g)));
	return id;
}

void World::draw(Win &win) {
	planes_[current_plane_].draw(win);
	player_->draw(win);
}

void World::update(float dt) {
	for (auto &plane: planes_)
		plane.update(dt);

	player_->update(planes_[current_plane_], dt);
}

void World::tick() {
	for (auto &plane: planes_)
		plane.tick();
}

}
