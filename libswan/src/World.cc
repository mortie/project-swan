#include "World.h"

namespace Swan {

WorldPlane::PlaneID World::addPlane() {
	WorldPlane::PlaneID id = planes_.size();
	planes_.push_back(WorldPlane());
	WorldPlane &plane = planes_.back();
	plane.id_ = id;
	return id;
}

void World::registerTile(Tile *t) {
	Tile::TileID id = registered_tiles_.size();
	registered_tiles_.push_back(t);
	tile_id_map_[t->name_] = id;
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
