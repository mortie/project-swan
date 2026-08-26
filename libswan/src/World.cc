#include "World.h"

#include <string_view>
#include <swan/constants.h>
#include <swan/log.h>

#include "Game.h"
#include "Clock.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep
#include "WorldData.h"

namespace Swan {

static void chunkLine(int l, WorldPlane &plane, ChunkPos &abspos, const Vec2i &dir)
{
	for (int i = 0; i < l; ++i) {
		plane.slowGetChunk(abspos).keepActive();
		abspos += dir;
	}
}

World::World(
		Game *game,
		uint32_t seed,
		WorldData data):
	game_(game),
	data_(std::move(data))
{}

/*
float World::findImageYOffset(ImageAsset &image)
{
	int y;
	bool done = false;
	for (y = 0; y < TILE_SIZE && !done; ++y) {
		unsigned char *row = &image.data[(TILE_SIZE - y - 1) * TILE_SIZE * 4];
		for (int x = 0; x < TILE_SIZE; ++x) {
			unsigned char *pix = &row[x * 4];
			if (pix[3] > 0) {
				done = true;
				break;
			}
		}
	}

	return (y - 1) / float(TILE_SIZE);
}
*/

void World::ChunkRenderer::tick(WorldPlane &plane, ChunkPos abspos)
{
	ZoneScopedN("World::ChunkRenderer tick");
	int l = 0;

	RTClock clock;
	for (int i = 0; i < 4; ++i) {
		chunkLine(l, plane, abspos, Vec2i(0, -1));
		chunkLine(l, plane, abspos, Vec2i(1, 0));
		l += 1;
		chunkLine(l, plane, abspos, Vec2i(0, 1));
		chunkLine(l, plane, abspos, Vec2i(-1, 0));
		l += 1;
	}
}

void World::setWorldGen(std::string gen)
{
	defaultWorldGen_ = std::move(gen);
}

void World::spawnPlayer()
{
	playerRef_ = planes_[currentPlane_].plane->spawnPlayer();
	player_ = playerRef_.trait<BodyTrait>();
}

void World::setCurrentPlane(WorldPlane &plane)
{
	currentPlane_ = plane.id_;
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

Cygnet::Color World::backgroundColor()
{
	auto &plane = planes_[currentPlane_].plane;
	return plane->worldGen_->backgroundColor(player_->pos);
}

void World::draw(Cygnet::Renderer &rnd)
{
	ZoneScopedN("World draw");
	planes_[currentPlane_].plane->draw(rnd, player_->center());
}

void World::update(float dt)
{
	ZoneScopedN("World update");
	for (auto &plane: planes_) {
		plane.plane->update(dt);
	}

	auto camTarget = player_->pos + player_->size / 2;
	auto camSqDist = (game_->cam_.pos - camTarget).squareLength();
	if (camSqDist > 20 * 20) {
		game_->cam_.pos = camTarget;
	} else {
		constexpr float HALF_LIFE = 0.05;
		game_->cam_.pos = {
			lerpSmooth(game_->cam_.pos.x, camTarget.x, HALF_LIFE, dt),
			lerpSmooth(game_->cam_.pos.y, camTarget.y, HALF_LIFE, dt),
		};
	}
}

bool World::tick(float dt, RTDeadline deadline)
{
	ZoneScopedN("World tick");

	if (!tickProgress_.ongoing) {
		chunkRenderer_.tick(
			*planes_[currentPlane_].plane,
			ChunkPos((int)player_->pos.x / CHUNK_WIDTH, (int)player_->pos.y / CHUNK_HEIGHT));

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

	playerRef_.serialize(w.initPlayer());
	w.setCurrentPlane(currentPlane_);
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

	currentPlane_ = r.getCurrentPlane();
	playerRef_.deserialize(currentPlane().getContext(), r.getPlayer());
	player_ = playerRef_.trait<BodyTrait>();
	if (!player_) {
		panic << "Missing player body!";
		throw std::runtime_error("Missing player body");
	}

	// Position camera
	game_->cam_.pos = player_->pos + player_->size / 2;
}

}
