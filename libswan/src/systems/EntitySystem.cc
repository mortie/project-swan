#include "systems/EntitySystem.h"

#include "WorldPlane.h"
#include "traits/TileEntityTrait.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep

namespace Swan {

EntitySystem::EntitySystem(
	WorldPlane &plane,
	std::vector<std::unique_ptr<EntityCollection>> &&colls):
	plane_(plane),
	collections_(std::move(colls))
{
	for (auto &coll: collections_) {
		collectionsByType_[coll->type()] = coll.get();
		collectionsByName_[coll->name()] = coll.get();
	}
}

EntityRef EntitySystem::spawn(std::string_view name, sbon::ObjectReader r)
{
	auto it = collectionsByName_.find(name);
	if (it == collectionsByName_.end()) {
		warn << "Reference to unregistered entity '" << name << "'";
		return {};
	}

	auto ctx = getContext();
	auto ent = it->second->spawn(ctx, r);
	ent->onSpawn(ctx);
	return ent;
}

void EntitySystem::despawn(EntityRef ref)
{
	despawnListA_.push_back(ref);
}

std::span<EntitySystem::FoundEntity> EntitySystem::getColliding(
	BodyTrait::Body &body)
{
	constexpr float PADDING = 10;
	auto topLeft = body.topLeft() - Vec2{PADDING, PADDING};
	auto bottomRight = body.bottomRight() + Vec2{PADDING, PADDING};

	auto topLeftTile = TilePos{(int)floor(topLeft.x), (int)floor(topLeft.y)};
	auto bottomRightTile = TilePos{(int)ceil(bottomRight.x), (int)ceil(bottomRight.y)};

	auto topLeftChunk = tilePosToChunkPos(topLeftTile);
	auto bottomRightChunk = tilePosToChunkPos(bottomRightTile);
	auto bottomLeftChunk = ChunkPos{topLeftChunk.x, bottomRightChunk.y};
	auto topRightChunk = ChunkPos{bottomRightChunk.x, topLeftChunk.y};

	foundEntitiesBuf_.clear();
	auto checkChunk = [&](ChunkPos pos) {
		Chunk &chunk = plane_.getChunk(pos);
		for (const EntityRef &constCandidate: chunk.entities_) {
			EntityRef candidateRef = constCandidate;

			// The entities_ array in a chunk should always be kept updated
			assert(candidateRef);

			BodyTrait::Body *candidateBody = candidateRef.getBody();
			if (!candidateBody || candidateBody == &body) {
				continue;
			}

			if (body.collidesWith(*candidateBody)) {
				foundEntitiesBuf_.push_back({candidateRef, *candidateBody});
			}
		}
	};

	// TODO: I'm 99% sure we can do something better here

	checkChunk(topLeftChunk);

	if (bottomLeftChunk != topLeftChunk) {
		checkChunk(bottomLeftChunk);
	}

	if (bottomRightChunk != bottomLeftChunk && bottomRightChunk != topLeftChunk) {
		checkChunk(bottomRightChunk);
	}

	if (
		topRightChunk != bottomRightChunk && topRightChunk != bottomLeftChunk &&
		topRightChunk != topLeftChunk) {
		checkChunk(topRightChunk);
	}

	return foundEntitiesBuf_;
}

std::span<EntitySystem::FoundEntity> EntitySystem::getInTile(
	TilePos pos)
{
	BodyTrait::Body body = {
		.pos = pos,
		.size = {1, 1},
		.chunkPos = tilePosToChunkPos(pos),
	};

	return getColliding(body);
}

std::span<EntitySystem::FoundEntity> EntitySystem::getInArea(
	Vec2 pos, Vec2 size)
{
	BodyTrait::Body body = {
		.pos = pos,
		.size = size,
		.chunkPos = tilePosToChunkPos(pos.as<int>()),
	};

	return getColliding(body);
}

void EntitySystem::spawnTileEntity(TilePos pos, std::string_view name)
{
	if (tileEntities_.contains(pos)) {
		warn << "Tile entity already exists in " << pos;
		return;
	}

	auto it = collectionsByName_.find(name);
	if (it == collectionsByName_.end()) {
		warn << "Tile entity " << name << " doesn't exist";
		return;
	}

	auto ent = it->second->spawn(getContext());
	auto *tileEnt = ent.trait<TileEntityTrait>();
	if (tileEnt) {
		tileEnt->pos = pos;
	}

	tileEntities_[pos] = ent;
	ent->onSpawn(getContext());
}

EntityRef EntitySystem::getTileEntity(TilePos pos)
{
	auto it = tileEntities_.find(pos);
	if (it == tileEntities_.end()) {
		return {};
	} else {
		return it->second;
	}
}

void EntitySystem::despawnTileEntity(TilePos pos)
{
	auto it = tileEntities_.find(pos);

	if (it == tileEntities_.end()) {
		warn << "Didn't find expected tile entity at " << pos;
	}
	else {
		despawn(it->second);
		tileEntities_.erase(pos);
	}
}

EntityRef EntitySystem::current()
{
	if (!currentCollection_) {
		return {};
	}

	return currentCollection_->currentEntity();
}

Context EntitySystem::getContext() {
	return plane_.getContext();
}

void EntitySystem::draw(Cygnet::Renderer &rnd)
{
	auto ctx = getContext();
	for (auto &coll: collections_) {
		coll->draw(ctx, rnd);
	}
}

void EntitySystem::update(float dt)
{
	auto ctx = getContext();
	for (auto &coll: collections_) {
		currentCollection_ = coll.get();
		coll->update(ctx, dt);
	}
	currentCollection_ = nullptr;

	auto despawnList = std::move(despawnListA_);
	despawnListA_ = std::move(despawnListB_);

	for (auto &ref: despawnList) {
		if (ref) {
			ref->onDespawn(ctx);
		}

		ref.coll_->erase(ctx, ref.id_);
	}

	despawnList.clear();
	despawnListB_ = std::move(despawnList);
}

void EntitySystem::tick(float dt)
{
	auto ctx = getContext();

	for (auto &coll: collections_) {
		currentCollection_ = coll.get();
		coll->tick(ctx, dt);
	}

	for (auto &coll: collections_) {
		currentCollection_ = coll.get();
		coll->tick2(ctx, dt);
	}

	currentCollection_ = nullptr;
}

EntityCollection *EntitySystem::getCollectionOf(std::string_view name)
{
	auto it = collectionsByName_.find(name);
	if (it == collectionsByName_.end()) {
		warn << "Reference to unregistered entity collection '" << name << "'";
		return nullptr;
	}

	return it->second;
}

}
