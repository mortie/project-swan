#include "WorldPlane.h"

#include <limits>
#include <math.h>

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
	entColls_(std::move(colls)),
	lighting_(std::make_unique<LightServer>(*this))
{
	for (auto &coll: entColls_) {
		entCollsByType_[coll->type()] = coll.get();
		entCollsByName_[coll->name()] = coll.get();
	}
}

Context WorldPlane::getContext()
{
	return {
		.game = *world_->game_,
		.world = *world_,
		.plane = *this,
	};
}

EntityRef WorldPlane::spawnEntity(const std::string &name, MsgStream::MapParser &r)
{
	return entCollsByName_.at(name)->spawn(getContext(), r);
}

std::vector<WorldPlane::FoundEntity> &WorldPlane::getCollidingEntities(
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

	foundEntitiesRet_.clear();
	auto checkChunk = [&](ChunkPos pos) {
		Chunk &chunk = getChunk(pos);
		for (const EntityRef &constCandidate: chunk.entities_) {
			EntityRef candidateRef = constCandidate;

			// The entities_ array in a chunk should always be kept updated
			assert(candidateRef);

			BodyTrait::Body *candidateBody = candidateRef.getBody();
			if (!candidateBody || candidateBody == &body) {
				continue;
			}

			if (body.collidesWith(*candidateBody)) {
				foundEntitiesRet_.push_back({candidateRef, *candidateBody});
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

	return foundEntitiesRet_;
}

std::vector<WorldPlane::FoundEntity> &WorldPlane::getEntitiesInTile(TilePos pos)
{
	BodyTrait::Body body = {
		.pos = pos,
		.size = {1, 1},
		.chunkPos = tilePosToChunkPos(pos),
	};

	return getCollidingEntities(body);
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
		lighting_->onChunkAdded(pos, computeLightChunk(chunk));
	}

	// Otherwise, it might not be active, so let's activate it
	else if (!iter->second.isActive()) {
		iter->second.keepActive();
		activeChunks_.push_back(&iter->second);
		lighting_->onChunkAdded(pos, computeLightChunk(iter->second));
	}

	return iter->second;
}

void WorldPlane::setTileID(TilePos pos, Tile::ID id)
{
	setTileIDWithoutUpdate(pos, id);

	// Update the new tile immediately
	auto &tile = getTile(pos);
	if (tile.onTileUpdate) {
		tile.onTileUpdate(getContext(), pos);
	}

	// Schedule surrounding tile updates for later
	scheduledTileUpdates_.push_back(pos.add(-1, 0));
	scheduledTileUpdates_.push_back(pos.add(0, -1));
	scheduledTileUpdates_.push_back(pos.add(1, 0));
	scheduledTileUpdates_.push_back(pos.add(0, 1));
}

void WorldPlane::setTileIDWithoutUpdate(TilePos pos, Tile::ID id)
{
	Chunk &chunk = getChunk(tilePosToChunkPos(pos));
	ChunkRelPos rp = tilePosToChunkRelPos(pos);

	Tile::ID old = chunk.getTileID(rp);

	if (id == old) {
		return;
	}

	Tile &newTile = world_->getTileByID(id);
	Tile &oldTile = world_->getTileByID(old);

	if (oldTile.onBreak) {
		oldTile.onBreak(getContext(), pos);
	}

	if (oldTile.tileEntity) {
		auto it = tileEntities_.find(pos);
		if (it == tileEntities_.end()) {
			warn << "Didn't find expected tile entity at " << pos;
		}
		else {
			despawnEntity(it->second);
			tileEntities_.erase(pos);
		}
	}

	chunk.setTileID(rp, id);

	if (!oldTile.isOpaque && newTile.isOpaque) {
		lighting_->onSolidBlockAdded(pos);
	}
	else if (oldTile.isOpaque && !newTile.isOpaque) {
		lighting_->onSolidBlockRemoved(pos);
	}

	if (newTile.lightLevel != oldTile.lightLevel) {
		if (oldTile.lightLevel > 0) {
			removeLight(pos, oldTile.lightLevel);
		}

		if (newTile.lightLevel > 0) {
			addLight(pos, newTile.lightLevel);
		}
	}

	[&] {
		if (!newTile.tileEntity) {
			return;
		}

		if (tileEntities_.contains(pos)) {
			warn << "Tile entity already exists in " << pos;
			return;
		}

		auto it = entCollsByName_.find(newTile.tileEntity.value());
		if (it == entCollsByName_.end()) {
			warn
				<< "Tile entity " << newTile.tileEntity.value()
				<< " for tile " << newTile.name << " doesn't exist";
			return;
		}

		auto ent = it->second->spawn(getContext());
		auto *tileEnt = ent.trait<TileEntityTrait>();
		if (!tileEnt) {
			warn
				<< "Entity " << newTile.tileEntity.value()
				<< " doesn't implement TileEntityTrait";
			it->second->erase(getContext(), ent);
			return;
		}

		tileEnt->pos = pos;
		tileEntities_[pos] = ent;
	}();

	if (newTile.onSpawn) {
		newTile.onSpawn(getContext(), pos);
	}
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
	Tile::ID old = getTileID(pos);

	// If the block isn't air, do nothing
	if (old != World::AIR_TILE_ID) {
		return false;
	}

	Tile &tile = world_->getTileByID(id);

	// TODO: play a separate place sound
	world_->game_->playSound(tile.breakSound);

	// Change tile to air
	setTileID(pos, id);
	return true;
}

WorldPlane::Raycast WorldPlane::raycast(
	Vec2 start, Vec2 direction, float distance)
{
	float squareDist = distance * distance;
	Vec2 step = direction.norm() * 0.25;
	Vec2 pos = start;
	Vec2 prevPos = start;

	TilePos prevTP = {0, std::numeric_limits<int>::max()};
	TilePos tp = {(int)floor(pos.x), (int)floor(pos.y)};
	Tile *tile;

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
	std::lock_guard<std::mutex> lock(mut_);
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
					rnd.drawRect({chunk.pos_ * size, size, {0.7, 0.1, 0.2, 1}});
				}
			}
		}
	}

	for (auto &coll: entColls_) {
		coll->draw(ctx, rnd);
	}

	{
		ZoneScopedN("Lighting flip");
		lighting_->flip();
	}
}

void WorldPlane::ui()
{
	ZoneScopedN("WorldPlane ui");

	auto ctx = getContext();
	for (auto &coll: entColls_) {
		coll->ui(ctx);
	}
}

void WorldPlane::update(float dt)
{
	ZoneScopedN("WorldPlane update");
	std::lock_guard<std::mutex> lock(mut_);
	auto ctx = getContext();

	// Just init one chunk per frame
	if (chunkInitList_.size() > 0) {
		Chunk *chunk = chunkInitList_.front();
		chunkInitList_.pop_front();

		TilePos base = chunk->topLeft();
		for (int y = 0; y < CHUNK_HEIGHT; ++y) {
			for (int x = 0; x < CHUNK_WIDTH; ++x) {
				Tile::ID id = chunk->getTileID({x, y});
				Tile &tile = world_->getTileByID(id);
				if (tile.onSpawn) {
					tile.onSpawn(getContext(), base + Vec2i{x, y});
				}
			}
		}

		worldGen_->initializeChunk(ctx, *chunk);
	}

	for (auto &coll: entColls_) {
		currentEntCol_ = coll.get();
		coll->update(ctx, dt);
	}

	auto despawnList = std::move(entDespawnList_);
	entDespawnList_ = std::move(entDespawnListB_);

	for (auto &ref: despawnList) {
		if (ref) {
			ref->onDespawn(ctx);
		}

		ref.coll_->erase(ctx, ref.id_);
	}

	despawnList.clear();
	entDespawnListB_ = std::move(despawnList);
}

void WorldPlane::tick(float dt)
{
	ZoneScopedN("WorldPlane tick");
	std::lock_guard<std::mutex> lock(mut_);
	auto ctx = getContext();

	// Any chunk which has been in use since last tick should be kept alive
	for (std::pair<ChunkPos, Chunk *> &ch: tickChunks_) {
		ch.second->keepActive();
	}
	tickChunks_.clear();

	for (auto &coll: entColls_) {
		currentEntCol_ = coll.get();
		coll->tick(ctx, dt);
	}

	// Tick all chunks, figure out if any of them should be deleted or compressed
	size_t activeChunkIndex = 0;
	while (activeChunkIndex < activeChunks_.size()) {
		auto &chunk = activeChunks_[activeChunkIndex];
		auto action = chunk->tick(dt);

		switch (action) {
		case Chunk::TickAction::DEACTIVATE:
			info << "Compressing inactive modified chunk " << chunk->pos_;
			lighting_->onChunkRemoved(chunk->pos_);
			chunk->compress(world_->game_->renderer_);
			activeChunks_[activeChunkIndex] = activeChunks_.back();
			activeChunks_.pop_back();
			break;

		case Chunk::TickAction::DELETE:
			info << "Deleting inactive unmodified chunk " << chunk->pos_;
			lighting_->onChunkRemoved(chunk->pos_);
			chunk->destroy(world_->game_->renderer_);
			chunks_.erase(chunk->pos_);
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
}

void WorldPlane::addLight(TilePos pos, float level)
{
	getChunk(tilePosToChunkPos(pos));
	lighting_->onLightAdded(pos, level);
}

void WorldPlane::removeLight(TilePos pos, float level)
{
	getChunk(tilePosToChunkPos(pos));
	lighting_->onLightRemoved(pos, level);
}

void WorldPlane::onLightChunkUpdated(const LightChunk &chunk, ChunkPos pos)
{
	std::lock_guard<std::mutex> lock(mut_);
	Chunk &realChunk = getChunk(pos);

	realChunk.setLightData(chunk.lightLevels);
}

void WorldPlane::serialize(MsgStream::Serializer &w)
{
	auto ctx = getContext();
	auto map = w.beginMap(3);

	map.writeString("entity-collections");
	auto colls = map.beginMap(entColls_.size());
	for (auto &coll: entColls_) {
		colls.writeString(coll->name());
		coll->serialize(ctx, colls);
	}
	map.endMap(colls);

	size_t chunkCount = 0;
	for (auto &[_, chunk]: chunks_) {
		if (chunk.isModified()) {
			chunkCount += 1;
		}
	}

	map.writeString("chunks");
	auto chunks = map.beginArray(chunkCount);
	for (auto &[pos, chunk]: chunks_) {
		if (!chunk.isModified()) {
			continue;
		}

		auto chunkArr = chunks.beginArray(3);
		chunkArr.writeInt(pos.x);
		chunkArr.writeInt(pos.y);
		chunk.serialize(chunkArr);
		chunks.endArray(chunkArr);
	}
	map.endMap(chunks);

	map.writeString("tile-entities");
	auto tileEnts = map.beginArray(tileEntities_.size());
	for (auto &[pos, ref]: tileEntities_) {
		auto entArr = tileEnts.beginArray(3);
		entArr.writeInt(pos.x);
		entArr.writeInt(pos.y);
		ref.serialize(entArr);
		tileEnts.endArray(entArr);
	}
	map.endArray(tileEnts);

	w.endMap(map);
}

void WorldPlane::deserialize(MsgStream::Parser &r)
{
	auto ctx = getContext();
	auto map = r.nextMap();

	std::string key;

	while (map.nextKey(key)) {
		if (key == "entity-collections") {
			auto colls = map.nextMap();
			while (colls.nextKey(key)) {
				auto it = entCollsByName_.find(key);
				if (it == entCollsByName_.end()) {
					warn << "Deserialize unknown entity collection: " << key;
					map.skipNext();
					continue;
				}

				it->second->deserialize(ctx, colls);
			}
		}
		else if (key == "chunks") {
			auto chunks = map.nextArray();
			chunks_.clear();
			activeChunks_.clear();
			chunkInitList_.clear();

			while (chunks.hasNext()) {
				auto chunkArr = chunks.nextArray();

				int x = chunkArr.nextInt();
				int y = chunkArr.nextInt();
				ChunkPos pos{x, y};

				auto it = chunks_.emplace(pos, Chunk(pos)).first;
				Chunk &chunk = it->second;

				chunk.deserialize(chunkArr);
				if (chunk.isActive()) {
					lighting_->onChunkAdded(pos, computeLightChunk(chunk));
					activeChunks_.push_back(&chunk);
				}

				chunkArr.skipAll();
			}
		}
		else if (key == "tile-entities") {
			auto tileEnts = map.nextArray();
			while (tileEnts.hasNext()) {
				auto entArr = tileEnts.nextArray();
				TilePos pos;
				pos.x = entArr.nextInt();
				pos.y = entArr.nextInt();
				EntityRef ref;
				ref.deserialize(ctx, entArr);
				entArr.skipAll();

				if (!ref) {
					warn << "Reference to non-existent entity in " << pos;
					continue;
				}

				auto tileEnt = ref.trait<TileEntityTrait>();
				if (!tileEnt) {
					warn
						<< "Tile entity reference to entity which "
						<< "doesn't implement TileEntityTrait in " << pos;
					continue;
				}

				tileEnt->pos = pos;
				tileEntities_[pos] = ref;
			}
		}
		else {
			map.skipNext();
		}
	}
}

NewLightChunk WorldPlane::computeLightChunk(const Chunk &chunk)
{
	NewLightChunk lc;

	for (int y = 0; y < CHUNK_HEIGHT; ++y) {
		for (int x = 0; x < CHUNK_WIDTH; ++x) {
			Tile::ID id = chunk.getTileID({x, y});
			Tile &tile = world_->getTileByID(id);
			if (tile.isOpaque) {
				lc.blocks[y * CHUNK_HEIGHT + x] = true;
			}
			if (tile.lightLevel > 0) {
				lc.lightSources[{x, y}] = tile.lightLevel;
			}
		}
	}

	return lc;
}

}
