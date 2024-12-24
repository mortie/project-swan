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

EntityRef WorldPlane::spawnPlayer()
{
	return worldGen_->spawnPlayer(getContext());
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

	{
		ZoneScopedN("Fluids");
		fluidSystem_.draw(rnd);
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

	{
		ZoneScopedN("Fluids");
		fluidSystem_.update(dt);
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

	// First swap all the A and B buffers,
	// so that the current frame's stuff is in 'B'...
	std::swap(nextTickA_, nextTickB_);
	tileSystem_.beginTick();

	// Then run through the 'B' buffers of nextTick
	for (auto &cb: nextTickB_) {
		cb(ctx);
	}
	nextTickB_.clear();

	// ..and the 'B' buffers of the tile system
	tileSystem_.endTick();

	// Tick the rest
	{
		ZoneScopedN("Fluids");
		fluidSystem_.tick();
	}
	{
		ZoneScopedN("Entities");
		entitySystem_.tick(dt);
	}
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
