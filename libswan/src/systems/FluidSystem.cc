#include "systems/FluidSystem.h"
#include "common.h"
#include "WorldPlane.h"
#include "World.h"
#include "Game.h"
#include "cygnet/Renderer.h"

#include <climits>
#include <stdexcept>

namespace Swan {

namespace {

// Get the 2-logarithm of a power of 2
constexpr int64_t log2OfPow2(int64_t num)
{
	constexpr int64_t lim = (CHAR_BIT * sizeof(int64_t)) - 1;
	int64_t val = -1;
	for (int64_t i = 0; i < lim; ++i) {
		if (num & ((int64_t)1 << i)) {
			if (val >= 0) {
				throw std::runtime_error("log2OfPow2 called on non-power-of-2");
			}

			val = i;
		}
	}

	if (val < 0) {
		throw std::runtime_error("log2OfPow2 called on zero");
	}

	return val;
}

void fluidPosToWorldPos(FluidPos pos, ChunkPos &cpos, Vec2i &rel)
{
	constexpr int64_t FLUID_PER_CHUNK_X = CHUNK_WIDTH * FLUID_RESOLUTION;
	constexpr int64_t PER_CHUNK_X_BITS = log2OfPow2(FLUID_PER_CHUNK_X);
	constexpr int64_t PER_CHUNK_X_MASK = (1 << PER_CHUNK_X_BITS) - 1;
	constexpr int64_t FLUID_PER_CHUNK_Y = CHUNK_HEIGHT * FLUID_RESOLUTION;
	constexpr int64_t PER_CHUNK_Y_BITS = log2OfPow2(FLUID_PER_CHUNK_Y);
	constexpr int64_t PER_CHUNK_Y_MASK = (1 << PER_CHUNK_Y_BITS) - 1;

	cpos = {
		int((pos.x & ~PER_CHUNK_X_MASK) >> PER_CHUNK_X_BITS),
		int((pos.y & ~PER_CHUNK_Y_MASK) >> PER_CHUNK_Y_BITS),
	};

	rel = {
		int(pos.x & PER_CHUNK_X_MASK),
		int(pos.y & PER_CHUNK_Y_MASK),
	};
}

Vec2 fluidPosToWorldPos(FluidPos pos)
{
	ChunkPos cpos;
	Vec2i rel;
	fluidPosToWorldPos(pos, cpos, rel);

	return {
		cpos.x * CHUNK_WIDTH + (rel.x / float(FLUID_RESOLUTION)),
		cpos.y * CHUNK_HEIGHT + (rel.y / float(FLUID_RESOLUTION)),
	};
}

FluidPos worldPosToFluidPos(Vec2 pos)
{
	if (pos.x > 0) {
		pos.x += 1.0 / FLUID_RESOLUTION;
	}

	if (pos.y > 0) {
		pos.y += 1.0 / FLUID_RESOLUTION;
	}

	pos.x -= 0.5 / FLUID_RESOLUTION;
	pos.y -= 0.5 / FLUID_RESOLUTION;
	return (pos * FLUID_RESOLUTION).as<int64_t>();
}

}

void FluidSystemImpl::FluidCellRef::setAir()
{
	*value_ = 0;
}

bool FluidSystemImpl::FluidCellRef::isAir()
{
	return *value_ == 0;
}

bool FluidSystemImpl::FluidCellRef::isSolid()
{
	return *value_ == 1;
}

void FluidSystemImpl::FluidCellRef::set(Fluid::ID id, int vx)
{
	int mode;
	if (vx < 0) {
		mode = 1;
	}
	else if (vx > 0) {
		mode = 2;
	}
	else {
		mode = 0;
	}

	*value_ = (mode << 6) | int(id);
}

int FluidSystemImpl::FluidCellRef::vx()
{
	int mode = *value_ >> 6;
	if (mode == 1) {
		return -1;
	}
	else if (mode == 2) {
		return 1;
	}
	else {
		return 0;
	}
}

void FluidSystemImpl::FluidCellRef::setVX(int vx)
{
	int mode;
	if (vx < 0) {
		mode = 1;
	}
	else if (vx > 0) {
		mode = 2;
	}
	else {
		mode = 0;
	}

	*value_ = (*value_ & 0x3f) | (mode << 6);
}

Fluid::ID FluidSystemImpl::FluidCellRef::id()
{
	return Fluid::ID(*value_ & 0x3f);
}

void FluidSystemImpl::FluidCellRef::setID(Fluid::ID id)
{
	*value_ = (*value_ & 0xc0) | id;
}

void FluidSystemImpl::triggerUpdateInTile(TilePos tpos)
{
	FluidPos fpos = {
		tpos.x * FLUID_RESOLUTION - 1,
		tpos.y * FLUID_RESOLUTION - 1,
	};

	for (int64_t y = 0; y < FLUID_RESOLUTION + 2; ++y) {
		for (int64_t x = 0; x < FLUID_RESOLUTION + 2; ++x) {
			triggerUpdate(fpos.add(x, y));
		}
	}
}

void FluidSystemImpl::setInTile(TilePos pos, Fluid::ID fluid)
{
	auto chunkPos = tilePosToChunkPos(pos);
	auto relPos = tilePosToChunkRelPos(pos);
	auto &chunk = plane_.getChunk(chunkPos);

	uint8_t *data = chunk.getFluidData();
	for (size_t y = 0; y < FLUID_RESOLUTION; ++y) {
		uint8_t *row = &data[
			(relPos.y * FLUID_RESOLUTION + y) * CHUNK_WIDTH * FLUID_RESOLUTION];
		for (size_t x = 0; x < FLUID_RESOLUTION; ++x) {
			Fluid::ID id = row[relPos.x * FLUID_RESOLUTION + x] & 0x3f;
			if (id == World::AIR_FLUID_ID || id == World::SOLID_FLUID_ID) {
				continue;
			}

			float vx = (x - (FLUID_RESOLUTION / 2.0 - 0.5));
			float vy = (y - (FLUID_RESOLUTION / 2.0 - 0.5));

			particles_.push_back({
				.pos = pos.as<float>().add(
					float(x) / FLUID_RESOLUTION,
					float(y) / FLUID_RESOLUTION),
				.vel = {vx, vy},
				.color = plane_.world_->getFluidByID(id).color,
				.id = id,
				.remainingTime = 255,
			});
		}
	}

	chunk.setFluidID(relPos, fluid);
	triggerUpdateInTile(pos);
}

Fluid &FluidSystemImpl::getAtPos(Vec2 pos) {
	return plane_.world_->getFluidByID(getFluidCell(worldPosToFluidPos(pos)).id());
}

void FluidSystemImpl::draw(Cygnet::Renderer &rnd)
{
	for (auto &particle: particles_) {
		rnd.drawParticle({
			.pos = particle.pos,
			.size = {1.0 / FLUID_RESOLUTION, 1.0 / FLUID_RESOLUTION},
			.color = particle.color,
		});
	}

	if (plane_.world_->game_->debugFluidParticleLocations_) {
		for (auto &particle: particles_) {
			rnd.drawRect(Cygnet::Renderer::DrawRect{
				.pos = fluidPosToWorldPos(worldPosToFluidPos(particle.pos))
					.add(-0.05, -0.05)
					.add(0.5 / FLUID_RESOLUTION, 0.5 / FLUID_RESOLUTION),
				.size = {0.1, 0.1},
				.fill = {},
			});
		}
	}
}

void FluidSystemImpl::update(float dt)
{
	for (size_t i = 0; i < particles_.size();) {
		auto &particle = particles_[i];

		int vx = particle.vel.x < -0.1 ? -1 : particle.vel.x > 0.1 ? 1 : 0;
		int vy = particle.vel.y < -0.1 ? -1 : 1;

		FluidPos pos = worldPosToFluidPos(particle.pos);
		FluidCellRef nearbyX = getFluidCell(pos.add(vx, 0));
		FluidCellRef nearbyY = getFluidCell(pos.add(0, vy));

		if (nearbyX.isAir() && nearbyY.isAir()) {
			particle.vel += (particle.vel * -0.9) * dt;
			particle.vel.y += 20 * dt;
			particle.pos += particle.vel * dt;
			i += 1;
			continue;
		}

		FluidCellRef self = getFluidCell(pos);
		if (self.isAir()) {
			self.set(particle.id, vx);
			triggerUpdateAround(pos);
			particles_[i] = particles_.back();
			particles_.pop_back();
			continue;
		}

		if (nearbyX.isAir()) {
			nearbyX.set(particle.id, vx);
			triggerUpdateAround(pos.add(vx, 0));
			particles_[i] = particles_.back();
			particles_.pop_back();
			continue;
		}

		if (nearbyY.isAir()) {
			nearbyY.set(particle.id, vx);
			triggerUpdateAround(pos.add(0, vy));
			particles_[i] = particles_.back();
			particles_.pop_back();
			continue;
		}

		auto invNearbyY = getFluidCell(pos.add(0, -vy));
		if (invNearbyY.isAir()) {
			invNearbyY.set(particle.id, vx);
			triggerUpdateAround(pos.add(0, -vy));
			particles_[i] = particles_.back();
			particles_.pop_back();
			continue;
		}

		if (vx != 0 && nearbyX.isSolid()) {
			particle.vel.x *= -1;
		}

		FluidCellRef oppositeNearbyY = getFluidCell(pos.add(0, -vy));
		if (nearbyY.isSolid() && !oppositeNearbyY.isSolid()) {
			particle.vel.y *= -1;
		} else {
			particle.vel.y -= 5 * dt;
		}

		particle.pos += particle.vel * dt;
		i += 1;
	}
}

void FluidSystemImpl::tick()
{
	updateSet_.clear();
	movedSet_.clear();
	updatesB_.clear();
	std::swap(updatesA_, updatesB_);

	// Tick particles, and delete old ones
	for (size_t i = 0; i < particles_.size();) {
		if (particles_[i].remainingTime == 0) {
			particles_[i] = particles_.back();
			particles_.pop_back();
			continue;
		}

		if (random() % 2) {
			particles_[i].remainingTime -= 1;
		}
		i += 1;
	}

	// Randomize update order
	for (size_t i = 1; i < updatesB_.size(); ++i) {
		size_t newIndex = random() % (updatesB_.size() - i) + i;
		if (i != newIndex) {
			std::swap(updatesB_[i], updatesB_[newIndex]);
		}
	}

	// Run the updates
	for (size_t i = 0; i < updatesB_.size(); ++i) {
		applyRules(updatesB_[i]);
	}
}

void FluidSystemImpl::serialize(proto::FluidSystem::Builder w)
{
	auto updatesW = w.initUpdates(updatesA_.size());
	for (size_t i = 0; i < updatesA_.size(); ++i) {
		updatesW[i].setX(updatesA_[i].x);
		updatesW[i].setY(updatesA_[i].y);
	}

	auto particlesW = w.initParticles(particles_.size());
	for (size_t i = 0; i < particles_.size(); ++i) {
		particlesW[i].setId(particles_[i].id);
		auto posW = particlesW[i].initPos();
		posW.setX(particles_[i].pos.x);
		posW.setY(particles_[i].pos.y);
		auto velW = particlesW[i].initVel();
		velW.setX(particles_[i].vel.x);
		velW.setY(particles_[i].vel.y);
	}
}

void FluidSystemImpl::deserialize(proto::FluidSystem::Reader r)
{
	updatesA_.clear();
	updatesA_.reserve(r.getUpdates().size());
	for (auto update: r.getUpdates()) {
		updatesA_.push_back({update.getX(), update.getY()});
	}

	particles_.clear();
	particles_.reserve(r.getParticles().size());
	for (auto particle: r.getParticles()) {
		auto pos = particle.getPos();
		auto vel = particle.getVel();
		particles_.push_back({
			.pos = {pos.getX(), pos.getY()},
			.vel = {vel.getX(), vel.getY()},
			.color = plane_.world_->getFluidByID(particle.getId()).color,
			.id = particle.getId(),
			.remainingTime = particle.getRemainingTime(),
		});
	}
}

void FluidSystemImpl::triggerUpdate(FluidPos pos)
{
	if (updateSet_.contains(pos)) {
		return;
	}

	updateSet_.insert(pos);
	updatesA_.push_back(pos);
}

void FluidSystemImpl::triggerUpdateAround(FluidPos pos)
{
	triggerUpdate(pos);
	triggerUpdate(pos.add(-1, -1));
	triggerUpdate(pos.add(0, -1));
	triggerUpdate(pos.add(1, -1));
	triggerUpdate(pos.add(-1, 0));
	triggerUpdate(pos.add(0, 0));
	triggerUpdate(pos.add(1, 0));
	triggerUpdate(pos.add(-1, 1));
	triggerUpdate(pos.add(0, 1));
	triggerUpdate(pos.add(1, 1));
}

void FluidSystemImpl::applyRules(FluidPos pos)
{
	if (movedSet_.contains(pos)) {
		return;
	}
	movedSet_.insert(pos);

	FluidCellRef self = getFluidCell(pos);
	Fluid::ID id = self.id();
	if (id <= World::SOLID_FLUID_ID || id >= World::INVALID_FLUID_ID) {
		return;
	}

	int vx = self.vx();

	auto belowPos = pos.add(0, 1);
	FluidCellRef below = getFluidCell(belowPos);
	if (below.isAir()) {
		triggerUpdateAround(pos);

		if (vx != 0) {
			FluidCellRef nearbyBelow = getFluidCell(belowPos.add(vx, 0));
			FluidCellRef nearby = getFluidCell(pos.add(vx, 0));
			if (nearbyBelow.isAir() && nearby.isAir()) {
				self.setAir();
				particles_.push_back({
					.pos = fluidPosToWorldPos(pos),
					.vel = {float(vx) * 5, 0},
					.color = plane_.world_->getFluidByID(id).color,
					.id = id,
					.remainingTime = 255,
				});
				return;
			}
		}

		auto below2Pos = belowPos.add(0, 1);
		FluidCellRef below2 = getFluidCell(below2Pos);
		if (below2.isAir()) {
			self.setAir();
			particles_.push_back({
				.pos = fluidPosToWorldPos(pos),
				.vel = {0, 5},
				.color = plane_.world_->getFluidByID(id).color,
				.id = id,
				.remainingTime = 255,
			});
			return;
		}

		triggerUpdateAround(belowPos);
		self.setAir();
		below.set(id, self.vx());
		movedSet_.insert(belowPos);
		return;
	}

	if (vx == 0) {
		// 1 or -1
		int ax = 1 - (random() % 2) * 2;
		int bx = -ax;

		auto aPos = pos.add(ax, 0);
		auto a = getFluidCell(aPos);
		if (a.isAir()) {
			self.setAir();
			a.set(id, ax);
			triggerUpdateAround(pos);
			triggerUpdateAround(aPos);
			return;
		}

		auto bPos = pos.add(bx, 0);
		auto b = getFluidCell(bPos);
		if (b.isAir()) {
			self.setAir();
			b.set(id, bx);
			triggerUpdateAround(pos);
			triggerUpdateAround(bPos);
			return;
		}

		return;
	}

	auto nearbyPos = pos.add(vx, 0);
	auto nearby = getFluidCell(nearbyPos);
	if (nearby.isAir()) {
		triggerUpdateAround(pos);
		triggerUpdateAround(nearbyPos);
		nearby.set(id, vx);
		self.setAir();
		movedSet_.insert(nearbyPos);
		return;
	}

	triggerUpdateAround(pos);
	self.setVX(0);
}

FluidSystemImpl::FluidCellRef FluidSystemImpl::getFluidCell(FluidPos pos)
{
	ChunkPos cpos;
	Vec2i rel;
	fluidPosToWorldPos(pos, cpos, rel);

	auto &chunk = plane_.getChunk(cpos);
	chunk.setFluidModified();
	return &chunk.getFluidData()[(rel.y * CHUNK_WIDTH * FLUID_RESOLUTION) + rel.x];
}

}
