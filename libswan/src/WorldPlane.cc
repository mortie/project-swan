#include "WorldPlane.h"

#include <limits>
#include <math.h>
#include <utility>

#include "log.h"
#include "World.h"
#include "Game.h"
#include "traits/TileEntityTrait.h"
#include "EntityCollectionImpl.h" // IWYU pragma: keep

namespace Swan {

WorldPlane::WorldPlane(
	ID id, World *world, std::unique_ptr<WorldGen> worldGen,
	std::vector<std::unique_ptr<EntityCollection>> &&colls):
	id_(id), world_(world), worldGen_(std::move(worldGen)),
	entitySystem_(*this, std::move(colls))
{}

Context WorldPlane::getContext()
{
	return {
		.game = *world_->game_,
		.world = *world_,
		.plane = *this,
	};
}

bool WorldPlane::hasChunk(ChunkPos pos)
{
	return chunks_.find(pos) != chunks_.end();
}

// This function will be a bit weird because it's a really fucking hot function.
Chunk &WorldPlane::getChunk(ChunkPos pos)
{
	// First, look through all chunks which have been in use this tick
	for (auto [chpos, chunk]: tickChunks_) {
		if (chpos == pos) {
			return *chunk;
		}
	}

	Chunk &chunk = slowGetChunk(pos);
	tickChunks_.push_back({pos, &chunk});
	return chunk;
}

Chunk &WorldPlane::slowGetChunk(ChunkPos pos)
{
	ZoneScopedN("WorldPlane slowGetChunk");
	auto iter = chunks_.find(pos);

	// Create chunk if that turns out to be necessary
	if (iter == chunks_.end()) {
		iter = chunks_.emplace(pos, Chunk(pos)).first;
		Chunk &chunk = iter->second;

		worldGen_->genChunk(*this, chunk);
		activeChunks_.push_back(&chunk);
		chunkInitList_.push_back(&chunk);

		// Need to tell the light engine too
		lightSystem_.addChunk(pos, chunk);
	}

	// Otherwise, it might not be active, so let's activate it
	else if (!iter->second.isActive()) {
		iter->second.keepActive();
		activeChunks_.push_back(&iter->second);
		lightSystem_.addChunk(pos, iter->second);
	}

	return iter->second;
}

void WorldPlane::setTileID(TilePos pos, Tile::ID id)
{
	if (setTileIDWithoutUpdate(pos, id)) {
		// Update the new tile immediately
		auto &tile = getTile(pos);
		if (tile.onTileUpdate) {
			tile.onTileUpdate(getContext(), pos);
		}

		// Schedule surrounding tile updates for later
		scheduleTileUpdate(pos.add(-1, 0));
		scheduleTileUpdate(pos.add(0, -1));
		scheduleTileUpdate(pos.add(1, 0));
		scheduleTileUpdate(pos.add(0, 1));
	}
}

bool WorldPlane::setTileIDWithoutUpdate(TilePos pos, Tile::ID id)
{
	Chunk &chunk = getChunk(tilePosToChunkPos(pos));
	ChunkRelPos rp = tilePosToChunkRelPos(pos);

	Tile::ID old = chunk.getTileID(rp);

	if (id == old) {
		return false;
	}

	Tile &newTile = world_->getTileByID(id);
	Tile &oldTile = world_->getTileByID(old);

	if (oldTile.onBreak) {
		oldTile.onBreak(getContext(), pos);
	}

	if (oldTile.tileEntity) {
		entitySystem_.despawnTileEntity(pos);
	}

	chunk.setTileID(rp, id);

	if (!oldTile.isOpaque && newTile.isOpaque) {
		lightSystem_.addSolidBlock(pos);
	}
	else if (oldTile.isOpaque && !newTile.isOpaque) {
		lightSystem_.removeSolidBlock(pos);
	}

	if (newTile.lightLevel != oldTile.lightLevel) {
		if (oldTile.lightLevel > 0) {
			lightSystem_.removeLight(pos, oldTile.lightLevel);
		}

		if (newTile.lightLevel > 0) {
			lightSystem_.addLight(pos, newTile.lightLevel);
		}
	}

	if (oldTile.isSolid && !newTile.isSolid) {
		setFluid(pos, World::AIR_FLUID_ID);
	}
	else if (!oldTile.isSolid && newTile.isSolid) {
		setFluid(pos, World::SOLID_FLUID_ID);
	}

	if (newTile.onSpawn) {
		newTile.onSpawn(getContext(), pos);
	}

	if (newTile.tileEntity) {
		entitySystem_.spawnTileEntity(pos, newTile.tileEntity.value());
	}

	return true;
}

void WorldPlane::setTile(TilePos pos, const std::string &name)
{
	setTileID(pos, world_->getTileID(name));
}

Tile::ID WorldPlane::getTileID(TilePos pos)
{
	return getChunk(tilePosToChunkPos(pos)).getTileID(tilePosToChunkRelPos(pos));
}

Tile &WorldPlane::getTile(TilePos pos)
{
	return world_->getTileByID(getTileID(pos));
}

EntityRef WorldPlane::spawnPlayer()
{
	return worldGen_->spawnPlayer(getContext());
}

bool WorldPlane::breakTile(TilePos pos)
{
	// If the block is already air, do nothing
	Tile::ID id = getTileID(pos);

	if (id == World::AIR_TILE_ID) {
		return false;
	}

	Tile &tile = world_->getTileByID(id);

	if (tile.breakSound) {
		world_->game_->playSound(tile.breakSound);
	}
	else {
		world_->game_->playSound(world_->getSound(World::THUD_SOUND_NAME));
	}

	// Change tile to air
	setTileID(pos, World::AIR_TILE_ID);
	return true;
}

bool WorldPlane::placeTile(TilePos pos, Tile::ID id)
{
	Chunk &chunk = getChunk(tilePosToChunkPos(pos));
	ChunkRelPos rp = tilePosToChunkRelPos(pos);

	Tile::ID old = chunk.getTileID(rp);
	auto &oldTile = world_->getTileByID(old);

	if (!oldTile.isReplacable) {
		return false;
	}

	if (id == old) {
		return false;
	}

	auto &newTileBeforeSpawn = world_->getTileByID(id);

	// Try to run the onSpawn immediately,
	// and revert if it returns false
	chunk.setTileID(rp, id);
	if (
		newTileBeforeSpawn.onSpawn &&
		!newTileBeforeSpawn.onSpawn(getContext(), pos)) {
		chunk.setTileID(rp, old);
		return false;
	}

	// The ID might've been changed after onSpawn
	id = chunk.getTileID(rp);
	auto &newTile = world_->getTileByID(id);

	world_->game_->playSound(oldTile.breakSound);

	// TODO: play a separate place sound
	world_->game_->playSound(newTile.breakSound);

	// We didn't run the onBreak and despawn tile entities yet,
	// so let's do that
	chunk.setTileID(rp, old);
	if (oldTile.onBreak) {
		oldTile.onBreak(getContext(), pos);
	}
	if (oldTile.tileEntity) {
		entitySystem_.despawnTileEntity(pos);
	}
	chunk.setTileID(rp, id);

	if (!oldTile.isOpaque && newTile.isOpaque) {
		lightSystem_.addSolidBlock(pos);
	}
	else if (oldTile.isOpaque && !newTile.isOpaque) {
		lightSystem_.removeSolidBlock(pos);
	}

	if (newTile.lightLevel != oldTile.lightLevel) {
		if (oldTile.lightLevel > 0) {
			lightSystem_.removeLight(pos, oldTile.lightLevel);
		}

		if (newTile.lightLevel > 0) {
			lightSystem_.addLight(pos, newTile.lightLevel);
		}
	}

	if (oldTile.isSolid && !newTile.isSolid) {
		setFluid(pos, World::AIR_FLUID_ID);
	}
	else if (!oldTile.isSolid && newTile.isSolid) {
		setFluid(pos, World::SOLID_FLUID_ID);
	}

	if (newTile.tileEntity) {
		entitySystem_.spawnTileEntity(pos, newTile.tileEntity.value());
	}

	// Run a tile update immediately
	if (newTile.onTileUpdate) {
		newTile.onTileUpdate(getContext(), pos);
	}

	// Schedule surrounding tile updates for later
	scheduleTileUpdate(pos.add(-1, 0));
	scheduleTileUpdate(pos.add(0, -1));
	scheduleTileUpdate(pos.add(1, 0));
	scheduleTileUpdate(pos.add(0, 1));

	return true;
}

Raycast WorldPlane::raycast(
	Vec2 start, Vec2 direction, float distance)
{
	float squareDist = distance * distance;
	Vec2 step = direction.norm() * 0.25;
	Vec2 pos = start;
	Vec2 prevPos = start;

	TilePos prevTP = {0, std::numeric_limits<int>::max()};
	TilePos tp = {(int)floor(pos.x), (int)floor(pos.y)};
	Tile *tile = &getTile(tp);

	auto isFaceValid = [&](TilePos tp, Vec2i face) {
		tp += face;
		return !getTile(tp).isSolid;
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

		tile = &getTile(tp);
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

Cygnet::Color WorldPlane::backgroundColor()
{
	return worldGen_->backgroundColor(world_->player_->pos);
}

void WorldPlane::draw(Cygnet::Renderer &rnd)
{
	ZoneScopedN("WorldPlane draw");
	auto ctx = getContext();
	auto &pbody = *(world_->player_);

	{
		ZoneScopedN("Draw background");
		worldGen_->drawBackground(ctx, rnd, pbody.pos);
	}

	ChunkPos pcpos = ChunkPos(
		(int)floor(pbody.pos.x / CHUNK_WIDTH),
		(int)floor(pbody.pos.y / CHUNK_HEIGHT));

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			auto iter = chunks_.find(pcpos + ChunkPos(x, y));
			if (iter != chunks_.end()) {
				ZoneScopedN("Chunk");
				Chunk &chunk = iter->second;
				chunk.draw(ctx, rnd);

				if (ctx.game.debugDrawChunkBoundaries_) {
					Vec2i size = {CHUNK_WIDTH, CHUNK_HEIGHT};
					rnd.drawRect({chunk.pos() * size, size, {0.7, 0.1, 0.2, 1}});
				}
			}
		}
	}

	{
		ZoneScopedN("Entities");
		entitySystem_.draw(rnd);
	}

	{
		ZoneScopedN("Lighting flip");
		lightSystem_.flip();
	}
}

void WorldPlane::update(float dt)
{
	ZoneScopedN("WorldPlane update");

	// Just init one chunk per frame
	if (chunkInitList_.size() > 0) {
		Chunk *chunk = chunkInitList_.front();
		chunkInitList_.pop_front();

		TilePos base = chunk->topLeft();
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			for (int x = 0; x < CHUNK_WIDTH; ++x) {
				Tile::ID id = chunk->getTileID({x, y});
				Tile &tile = world_->getTileByID(id);

				if (tile.isSolid) {
					chunk->setFluidID({x, y}, World::SOLID_FLUID_ID);
				}

				if (tile.onSpawn) {
					tile.onSpawn(getContext(), base + Vec2i{x, y});
				}
			}
		}
	}

	{
		ZoneScopedN("Entities");
		entitySystem_.update(dt);
	}
}

void WorldPlane::tick(float dt)
{
	ZoneScopedN("WorldPlane tick");
	auto ctx = getContext();

	// Any chunk which has been in use since last tick should be kept alive
	for (std::pair<ChunkPos, Chunk *> &ch: tickChunks_) {
		ch.second->keepActive();
	}
	tickChunks_.clear();

	entitySystem_.tick(dt);

	// Tick all chunks, figure out if any of them should be deleted or compressed
	size_t activeChunkIndex = 0;
	while (activeChunkIndex < activeChunks_.size()) {
		auto &chunk = activeChunks_[activeChunkIndex];
		auto action = chunk->tick(dt);

		switch (action) {
		case Chunk::TickAction::DEACTIVATE:
			info << "Compressing inactive modified chunk " << chunk->pos();
			lightSystem_.removeChunk(chunk->pos());
			chunk->destroyTextures(world_->game_->renderer_);
			chunk->compress();
			activeChunks_[activeChunkIndex] = activeChunks_.back();
			activeChunks_.pop_back();
			break;

		case Chunk::TickAction::DELETE:
			info << "Deleting inactive unmodified chunk " << chunk->pos();
			lightSystem_.removeChunk(chunk->pos());
			chunk->destroyTextures(world_->game_->renderer_);
			chunks_.erase(chunk->pos());
			activeChunks_[activeChunkIndex] = activeChunks_.back();
			activeChunks_.pop_back();
			break;

		case Chunk::TickAction::NOTHING:
			activeChunkIndex += 1;
			break;
		}
	}

	// Tick callbacks and tile updates which end up scheduling more callbacks/updates
	// shouldn't have their callbacks/updates executed until next tick
	auto nextTick = std::move(nextTick_);
	nextTick_ = std::move(nextTickB_);
	auto scheduledTileUpdates = std::move(scheduledTileUpdates_);
	scheduledTileUpdates_ = std::move(scheduledTileUpdatesB_);

	// Run next tick callbacks
	for (auto &cb: nextTick) {
		cb(ctx);
	}
	nextTick.clear();
	nextTickB_ = std::move(nextTick);

	// Run tile updates
	for (auto &pos: scheduledTileUpdates) {
		auto &tile = getTile(pos);
		if (tile.onTileUpdate) {
			tile.onTileUpdate(ctx, pos);
		}
	}
	scheduledTileUpdates.clear();
	scheduledTileUpdatesB_ = std::move(scheduledTileUpdates);

	fluidSystem_.tick();
}

void WorldPlane::serialize(sbon::Writer w)
{
	w.writeObject([&](sbon::ObjectWriter w) {
		entitySystem_.serialize(w.key("entity-system"));

		w.key("chunks").writeArray([&](sbon::Writer w) {
			for (auto &[pos, chunk]: chunks_) {
				if (!chunk.isModified()) {
					continue;
				}

				chunk.serialize(w);
			}
		});
	});
}

void WorldPlane::deserialize(sbon::Reader r, std::span<Tile::ID> tileMap)
{
	r.readObject([&](std::string &key, sbon::Reader val) {
		if (key == "entity-system") {
			entitySystem_.deserialize(val);
		}
		else if (key == "chunks") {
			chunks_.clear();
			activeChunks_.clear();
			chunkInitList_.clear();
			val.readArray([&](sbon::Reader r) {
				Chunk tempChunk({0, 0});
				tempChunk.deserialize(r, tileMap);
				auto [it, _] = chunks_.emplace(
					tempChunk.pos(), std::move(tempChunk));
				auto &chunk = it->second;

				if (chunk.isActive()) {
					lightSystem_.addChunk(chunk.pos(), chunk);
					activeChunks_.push_back(&chunk);
				}
			});
		}
		else {
			val.skip();
		}
	});
}

}
