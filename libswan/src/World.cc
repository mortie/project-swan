#include "World.h"

namespace Swan {

void World::draw(Win &win) {
	for (WorldPlane *plane: planes_)
		plane->draw(win);

	player_->draw(win);
}

void World::update(float dt) {
	for (WorldPlane *plane: planes_)
		plane->update(dt);

	player_->update(dt);
}

void World::tick() {
	for (WorldPlane *plane: planes_)
		plane->tick();
}

}
