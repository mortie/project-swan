#include "systems/TileSystem.h"

#include "WorldPlane.h"
#include "World.h"
#include "Game.h"

namespace Swan {

void TileSystem::set(TilePos pos, const std::string &name)
{
	setID(pos, plane_.world_->getTileID(name));
}

void TileSystem::setID(TilePos pos, Tile::ID id)
{
	if (setIDWithoutUpdate(pos, id)) {
		// Update the new tile immediately
		auto &tile = get(pos);
		if (tile.onTileUpdate) {
			tile.onTileUpdate(plane_.getContext(), pos);
		}

		// Schedule surrounding tile updates for later
		scheduleUpdate(pos.add(-1, 0));
		scheduleUpdate(pos.add(0, -1));
		scheduleUpdate(pos.add(1, 0));
		scheduleUpdate(pos.add(0, 1));
	}
}

bool TileSystem::setIDWithoutUpdate(TilePos pos, Tile::ID id)
{
	Chunk &chunk = plane_.getChunk(tilePosToChunkPos(pos));
	ChunkRelPos rp = tilePosToChunkRelPos(pos);

	Tile::ID old = chunk.getTileID(rp);

	if (id == old) {
		return false;
	}

	Tile &newTile = plane_.world_->getTileByID(id);
	Tile &oldTile = plane_.world_->getTileByID(old);

	if (oldTile.onBreak) {
		oldTile.onBreak(plane_.getContext(), pos);
	}

	if (oldTile.tileEntity) {
		plane_.entities().despawnTileEntity(pos);
	}

	chunk.setTileID(rp, id);

	if (!oldTile.isOpaque && newTile.isOpaque) {
		plane_.lights().addSolidBlock(pos);
	}
	else if (oldTile.isOpaque && !newTile.isOpaque) {
		plane_.lights().removeSolidBlock(pos);
	}

	if (newTile.lightLevel != oldTile.lightLevel) {
		if (oldTile.lightLevel > 0) {
			plane_.lights().removeLight(pos, oldTile.lightLevel);
		}

		if (newTile.lightLevel > 0) {
			plane_.lights().addLight(pos, newTile.lightLevel);
		}
	}

	if (oldTile.isSolid && !newTile.isSolid) {
		plane_.setFluid(pos, World::AIR_FLUID_ID);
	}
	else if (!oldTile.isSolid && newTile.isSolid) {
		plane_.setFluid(pos, World::SOLID_FLUID_ID);
	}

	if (newTile.onSpawn) {
		newTile.onSpawn(plane_.getContext(), pos);
	}

	if (newTile.tileEntity) {
		plane_.entities().spawnTileEntity(pos, newTile.tileEntity.value());
	}

	return true;
}

Tile &TileSystem::get(TilePos pos)
{
	return plane_.world_->getTileByID(getID(pos));
}

Tile::ID TileSystem::getID(TilePos pos)
{
	Chunk &chunk = plane_.getChunk(tilePosToChunkPos(pos));
	ChunkRelPos rp = tilePosToChunkRelPos(pos);
	return chunk.getTileID(rp);
}

bool TileSystem::breakTile(TilePos pos)
{
	// If the block is already air, do nothing
	Tile::ID id = getID(pos);

	if (id == World::AIR_TILE_ID) {
		return false;
	}

	Tile &tile = plane_.world_->getTileByID(id);

	if (tile.breakSound) {
		plane_.world_->game_->playSound(tile.breakSound);
	}
	else {
		plane_.world_->game_->playSound(plane_.world_->getSound(World::THUD_SOUND_NAME));
	}

	// Change tile to air
	setID(pos, World::AIR_TILE_ID);
	return true;
}

bool TileSystem::placeTile(TilePos pos, Tile::ID id)
{
	Chunk &chunk = plane_.getChunk(tilePosToChunkPos(pos));
	ChunkRelPos rp = tilePosToChunkRelPos(pos);

	Tile::ID old = chunk.getTileID(rp);
	auto &oldTile = plane_.world_->getTileByID(old);

	if (!oldTile.isReplacable) {
		return false;
	}

	if (id == old) {
		return false;
	}

	auto &newTileBeforeSpawn = plane_.world_->getTileByID(id);

	// Try to run the onSpawn immediately,
	// and revert if it returns false
	chunk.setTileID(rp, id);
	if (
		newTileBeforeSpawn.onSpawn &&
		!newTileBeforeSpawn.onSpawn(plane_.getContext(), pos)) {
		chunk.setTileID(rp, old);
		return false;
	}

	// The ID might've been changed after onSpawn
	id = chunk.getTileID(rp);
	auto &newTile = plane_.world_->getTileByID(id);

	plane_.world_->game_->playSound(oldTile.breakSound);

	// TODO: play a separate place sound
	plane_.world_->game_->playSound(newTile.breakSound);

	// We didn't run the onBreak and despawn tile entities yet,
	// so let's do that
	chunk.setTileID(rp, old);
	if (oldTile.onBreak) {
		oldTile.onBreak(plane_.getContext(), pos);
	}
	if (oldTile.tileEntity) {
		plane_.entities().despawnTileEntity(pos);
	}
	chunk.setTileID(rp, id);

	if (!oldTile.isOpaque && newTile.isOpaque) {
		plane_.lights().addSolidBlock(pos);
	}
	else if (oldTile.isOpaque && !newTile.isOpaque) {
		plane_.lights().removeSolidBlock(pos);
	}

	if (newTile.lightLevel != oldTile.lightLevel) {
		if (oldTile.lightLevel > 0) {
			plane_.lights().removeLight(pos, oldTile.lightLevel);
		}

		if (newTile.lightLevel > 0) {
			plane_.lights().addLight(pos, newTile.lightLevel);
		}
	}

	if (oldTile.isSolid && !newTile.isSolid) {
		plane_.setFluid(pos, World::AIR_FLUID_ID);
	}
	else if (!oldTile.isSolid && newTile.isSolid) {
		plane_.setFluid(pos, World::SOLID_FLUID_ID);
	}

	if (newTile.tileEntity) {
		plane_.entities().spawnTileEntity(pos, newTile.tileEntity.value());
	}

	// Run a tile update immediately
	if (newTile.onTileUpdate) {
		newTile.onTileUpdate(plane_.getContext(), pos);
	}

	// Schedule surrounding tile updates for later
	scheduleUpdate(pos.add(-1, 0));
	scheduleUpdate(pos.add(0, -1));
	scheduleUpdate(pos.add(1, 0));
	scheduleUpdate(pos.add(0, 1));

	return true;
}

Raycast TileSystem::raycast(
	Vec2 start, Vec2 direction, float distance)
{
	float squareDist = distance * distance;
	Vec2 step = direction.norm() * 0.25;
	Vec2 pos = start;
	Vec2 prevPos = start;

	TilePos prevTP = {0, std::numeric_limits<int>::max()};
	TilePos tp = {(int)floor(pos.x), (int)floor(pos.y)};
	Tile *tile = &get(tp);

	auto isFaceValid = [&](TilePos tp, Vec2i face) {
		tp += face;
		return !get(tp).isSolid;
	};

	while (true) {
		do {
			prevPos = pos;
			pos += step;
			if ((pos - start).squareLength() > squareDist) {
				return {
					.hit = false,
					.tile = *tile,
					.pos = tp,
					.face = Vec2i::ZERO,
				};
			}

			tp = {(int)floor(pos.x), (int)floor(pos.y)};
		} while (tp == prevTP);
		prevTP = tp;

		tile = &get(tp);
		if (!tile->isSolid) {
			continue;
		}

		Vec2i face = Vec2i::ZERO;
		Vec2 rel = pos - tp;
		Vec2 prevRel = prevPos - tp;

		if (rel.y > 0 && prevRel.y < 0 && isFaceValid(tp, {0, -1})) {
			face = {0, -1};
		}
		else if (rel.y < 1 && prevRel.y > 1 && isFaceValid(tp, {0, 1})) {
			face = {0, 1};
		}
		else if (rel.x > 0 && prevRel.x < 0 && isFaceValid(tp, {-1, 0})) {
			face = {-1, 0};
		}
		else if (rel.x < 1 && prevRel.x > 1 && isFaceValid(tp, {1, 0})) {
			face = {1, 0};
		}

		return {
			.hit = false,
			.tile = *tile,
			.pos = tp,
			.face = face,
		};
	}
}

void TileSystem::beginTick()
{
	std::swap(scheduledUpdatesA_, scheduledUpdatesB_);
}

void TileSystem::endTick()
{
	auto ctx = plane_.getContext();
	for (auto &pos: scheduledUpdatesB_) {
		auto &tile = get(pos);
		if (tile.onTileUpdate) {
			tile.onTileUpdate(ctx, pos);
		}
	}

	scheduledUpdatesB_.clear();
}

}
