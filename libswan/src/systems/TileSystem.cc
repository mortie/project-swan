#include "systems/TileSystem.h"

#include "WorldPlane.h"
#include "World.h"
#include "Game.h"

namespace Swan {

void TileSystemImpl::set(TilePos pos, std::string_view name)
{
	setID(pos, plane_.world_->getTileID(name));
}

void TileSystemImpl::setID(TilePos pos, Tile::ID id)
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

bool TileSystemImpl::setIDWithoutUpdate(TilePos pos, Tile::ID id)
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
		plane_.fluids().setInTile(pos, World::AIR_FLUID_ID);
	}
	else if (!oldTile.isSolid && newTile.isSolid) {
		plane_.fluids().setInTile(pos, World::SOLID_FLUID_ID);
	}

	if (newTile.onSpawn) {
		newTile.onSpawn(plane_.getContext(), pos);
	}

	if (newTile.tileEntity) {
		plane_.entities().spawnTileEntity(pos, newTile.tileEntity.value());
	}

	return true;
}

Tile &TileSystemImpl::get(TilePos pos)
{
	return plane_.world_->getTileByID(getID(pos));
}

Tile::ID TileSystemImpl::getID(TilePos pos)
{
	Chunk &chunk = plane_.getChunk(tilePosToChunkPos(pos));
	ChunkRelPos rp = tilePosToChunkRelPos(pos);
	return chunk.getTileID(rp);
}

uint8_t TileSystemImpl::getLightLevel(TilePos pos)
{
	Chunk &chunk = plane_.getChunk(tilePosToChunkPos(pos));
	ChunkRelPos rp = tilePosToChunkRelPos(pos);
	return chunk.getLightLevel(rp);
}

bool TileSystemImpl::breakTile(TilePos pos)
{
	// If the block is already air, do nothing
	Tile::ID id = getID(pos);

	if (id == World::AIR_TILE_ID) {
		return false;
	}

	Tile &tile = plane_.world_->getTileByID(id);
	spawnTileParticles(pos, tile);

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

bool TileSystemImpl::placeTile(TilePos pos, Tile::ID id)
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
	plane_.world_->game_->playSound(newTile.placeSound);

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
		plane_.fluids().setInTile(pos, World::AIR_FLUID_ID);
	}
	else if (!oldTile.isSolid && newTile.isSolid) {
		plane_.fluids().setInTile(pos, World::SOLID_FLUID_ID);
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

Raycast TileSystemImpl::raycast(
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

void TileSystemImpl::spawnTileParticles(TilePos pos, const Tile &tile)
{
	// We normally want the particles to be drawn in front of tiles,
	// which means using layer NORMAL.
	// However, we also want it to be drawn behind fluids.
	// Therefore, if the tile is in a fluid, braw it in layer BEHIND.
	auto &fluid = plane_.fluids().getAtPos(pos.as<float>().add(0.5, 0.5));
	auto layer = fluid.id > World::SOLID_FLUID_ID
		? Cygnet::RenderLayer::BEHIND 
		: Cygnet::RenderLayer::NORMAL;

	for (int y = 0; y < 8; ++y) {
		float fy = pos.y + (y / 8.0);
		for (int x = 0; x < 8; ++x) {
			float fx = pos.x + (x / 8.0);

			plane_.world_->game_->spawnParticle(layer, {
				.pos = {fx, fy},
				.vel = {
					(randfloat() - 0.5f) * 2.0f,
					-randfloat() * 2.0f,
				},
				.color = tile.particles[y][x],
				.lifetime = (randfloat() * 0.5f) + 0.1f,
			});
		}
	}
}

void TileSystemImpl::beginTick()
{
	std::swap(scheduledUpdatesA_, scheduledUpdatesB_);
}

void TileSystemImpl::endTick()
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
