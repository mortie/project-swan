#include "World.h"

#include <string_view>
#include <swan/constants.h>
#include <swan/log.h>

#include "Game.h"
#include "Clock.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep
#include "WorldData.h"

namespace Swan {

World::World(
		Game *game,
		uint32_t seed,
		WorldData data):
	game_(game),
	data_(std::move(data))
{}

void World::setWorldGen(std::string gen)
{
	defaultWorldGen_ = std::move(gen);
}

WorldPlane &World::addPlane(std::string gen)
{
	WorldPlane::ID id = planes_.size();
	auto it = data_.worldGenFactories_.find(gen);

	if (it == data_.worldGenFactories_.end()) {
		panic << "Tried to add plane with non-existent world gen " << gen << "!";
		abort();
	}

	std::vector<std::unique_ptr<EntityCollection>> colls;
	colls.reserve(data_.entCollFactories_.size());
	for (auto &fact: data_.entCollFactories_) {
		colls.emplace_back(fact.second.create(fact.second.name));
	}

	WorldGen::Factory &factory = it->second;
	std::unique_ptr<WorldGen> g = factory.create(data_, seed_);
	planes_.push_back({
		.worldGen = std::move(gen),
		.plane = std::make_unique<WorldPlane>(
			id, &data_, game_,
			std::move(g), std::move(colls)),
	});
	return *planes_[id].plane;
}

void World::update(float dt)
{
	ZoneScopedN("World update");
	for (auto &plane: planes_) {
		plane.plane->update(dt);
	}
}

bool World::tick(float dt, RTDeadline deadline)
{
	ZoneScopedN("World tick");

	if (!tickProgress_.ongoing) {
		resourceTickCounter_ += 1;
		if (resourceTickCounter_ >= 2) {
			data_.resources_.tick();
			resourceTickCounter_ = 0;
		}
	}

	bool allPlanesTicked = true;
	for (auto &plane: planes_) {
		if (tickProgress_.tickedPlanes.contains(plane.plane->id_)) {
			continue;
		}

		if (!plane.plane->tick(dt, deadline)) {
			allPlanesTicked = false;
			break;
		}

		tickProgress_.tickedPlanes.insert(plane.plane->id_);
	}

	if (allPlanesTicked) {
		tickProgress_.tickedPlanes.clear();
		tickProgress_.ongoing = false;
		return true;
	} else {
		tickProgress_.ongoing = true;
		return false;
	}
}

void World::tickDone()
{
	for (auto &plane: planes_) {
		plane.plane->tickDone();
	}
}

void World::serialize(proto::World::Builder w)
{
	auto planesBuilder = w.initPlanes(planes_.size());
	for (size_t i = 0; i < planes_.size(); ++i) {
		planes_[i].plane->serialize(planesBuilder[i]);
		planesBuilder[i].setWorldGen(planes_[i].worldGen);
	}

	auto namesByID = w.initNamesByID(data_.namesByID_.size());
	for (size_t i = 0; i < data_.namesByID_.size(); ++i) {
		namesByID.set(i, data_.namesByID_[i]);
	}

	w.setSeed(seed_);
}

void World::deserialize(proto::World::Reader r)
{
	// Seed must exist before we deserialize planes
	seed_ = r.getSeed();

	auto planes = r.getPlanes();
	planes_.clear();
	planes_.reserve(planes.size());
	for (auto plane: planes) {
		addPlane(plane.getWorldGen().cStr()).deserialize(plane);
	}
}

}
