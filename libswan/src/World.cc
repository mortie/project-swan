#include "World.h"

namespace Swan {

WorldPlane::ID World::addPlane() {
	WorldPlane::ID id = planes_.size();
	planes_.push_back(WorldPlane());
	WorldPlane &plane = planes_.back();
	plane.id_ = id;
	plane.world_ = this;
	return id;
}

void World::draw(Win &win) {
	planes_[current_plane_].draw(win);
	player_->draw(win);
}

void World::update(float dt) {
	for (auto &plane: planes_)
		plane.update(dt);

	player_->update(dt);
}

void World::tick() {
	for (auto &plane: planes_)
		plane.tick();
}

}
