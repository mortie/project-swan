#include "CopperWireEntity.h"
#include "cygnet/Renderer.h"
#include "swan/common.h"
#include "swan/constants.h"
#include "world/util.h"

namespace CoreMod {

static constexpr float MAX_LENGTH = 25;
static constexpr int RESOLUTION = 5; // Segments per meter
static constexpr float SEGMENT_LENGTH = 1.0f / float(RESOLUTION);
static constexpr float SPRING_COEF = 1500;
static constexpr float GRAVITY = 4;

static constexpr float TICK_HZ = 1500;

void CopperWireEntity::setEndPoint(Swan::Vec2 endPoint)
{
	assert(points_.size() >= 1);

	auto pointer = endPoint - points_.front();
	float length = pointer.length();
	if (length > MAX_LENGTH) {
		endPoint = points_.front() + pointer.norm() * MAX_LENGTH;
		length = MAX_LENGTH;
	}

	int numSegments = std::max(int(length * RESOLUTION), 1);
	int numPoints = numSegments + 1; // Fencepost

	// Construct new segments if there are too few.
	// The new segments start at the end of the last segment,
	// then go towards the end point.
	if (numPoints > points_.size()) {
		auto segmentPointer = endPoint - points_.back();
		auto segmentDir = segmentPointer.norm();
		while (numPoints > points_.size()) {
			points_.push_back(points_.back() + segmentDir * SEGMENT_LENGTH);
		}
	} else if (numPoints < points_.size()) {
		points_.resize(numPoints);
	}

	// Always fix the last point to the end
	points_.back() = endPoint;
}

void CopperWireEntity::update(Swan::Ctx &ctx, float dt)
{
	if (points_.size() < 2) {
		Swan::warn << "Invalid copper wire entity with 0 points! Despawning.";
		ctx.plane.entities().despawn(ctx.plane.entities().current());
		return;
	}

	assert(points_.size() >= 2);

	timer_ += dt;
	while (timer_ > 0) {
		simulatePhysicsStep(ctx, 1.0 / TICK_HZ);
		timer_ -= 1.0 / TICK_HZ;
	}

	performCollisions(ctx);
}

void CopperWireEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	rnd.drawPolyLine({
		.points = points_,
		.width = 0.05,
		.padding = 0.025,
		.fill = {0.96, 0.54, 0.17},
	});

	if (points_.size() < 3) {
		return;
	}

	rnd.drawPolyLine({
		.points = std::span(points_.data() + 1, points_.size() - 2),
		.width = 0.1,
		.fill = {0.1, 0.1, 0.1},
	});
}

void CopperWireEntity::onDespawn(Swan::Ctx &ctx)
{
	if (points_.empty()) {
		return;
	}

	dropItem(ctx, Swan::tilePos(points_[0]), "core::copper-wire");
	for (size_t i = 0; i < points_.size() - 1; ++i) {
		auto a = points_[i];
		auto b = points_[i + 1];

		Cygnet::Color color;
		if (i == 0 || i == points_.size() - 2) {
			color = {0.96, 0.54, 0.17};
		} else {
			color = {0.1, 0.1, 0.1};
		}

		for (int i = 0; i < 10; ++i) {
			Swan::Vec2 pos = {
				Swan::lerp(a.x, b.x, i / 10.0),
				Swan::lerp(a.y, b.y, i / 10.0),
			};
			ctx.game.spawnParticle({
				.pos = pos.add(-0.025, -0.025),
				.vel = {
					Swan::randfloat() - 0.5f,
					0.0,
				},
				.size = {0.05, 0.05},
				.color = color,
				.lifetime = Swan::randfloat() * 0.2f + 0.05f,
				.weight = 0.4,
			});
		}
	}
}

void CopperWireEntity::serialize(Swan::Ctx &ctx, Proto::Builder w)
{
	begin_.serialize(w.initBegin());
	end_.serialize(w.initEnd());

	auto points = w.initPoints(points_.size());
	for (size_t i = 0; i < points_.size(); ++i) {
		points[i].setX(points_[i].x);
		points[i].setY(points_[i].y);
	}
}

void CopperWireEntity::deserialize(Swan::Ctx &ctx, Proto::Reader r)
{
	begin_.deserialize(ctx, r.getBegin());
	end_.deserialize(ctx, r.getEnd());

	auto points = r.getPoints();
	points_.resize(points.size());
	for (size_t i = 0; i < points.size(); ++i) {
		points_[i].x = points[i].getX();
		points_[i].y = points[i].getY();
	}
}

void CopperWireEntity::simulatePhysicsStep(Swan::Ctx &ctx, float dt)
{
	// Keep a single static thread local vector of forces,
	// no need to keep one per copper wrie entity.
	// It keeps track of the forces we need to apply to every segment.
	// forces[i] corresponds to points_[i].
	// forces[0] and forces[points_.size() - 1] are technically unnecessary.
	static thread_local std::vector<Swan::Vec2> forces;
	if (forces.size() < points_.size()) {
		forces.resize(points_.size());
	}

	// Compute forces for non-end points.
	for (size_t i = 1; i < points_.size() - 1; ++i) {
		auto point = points_[i];
		auto left = points_[i - 1];
		auto right = points_[i + 1];
		auto &force = forces[i];

		// Compute the lengths of the left and right segments
		float leftLength = (point - left).length();
		float rightLength = (point - right).length();

		// Compute the error values for left and right segments:
		// a negative error means too short, a positive error means too long.
		// Divide by two because the point on the other side makes the same computation.
		// Note: this used to use the desired segment length,
		// but it turns out that just taking the desired length of a segment to be 0
		// works pretty well.
		float leftError = leftLength / 2;
		float rightError = rightLength / 2;

		// Compute force: move towards points which are too long,
		// and away from points which are too short
		force = (
			(left - point).norm() * leftError * SPRING_COEF +
			(right - point).norm() * rightError * SPRING_COEF);

		// Also, add gravity
		force.y += GRAVITY;
	}

	// Resolve forces
	for (size_t i = 0; i < points_.size() - 1; ++i) {
		auto delta = forces[i] * dt;
		if (delta.squareLength() > 1) {
			delta = delta.norm();
		}
		points_[i] += delta;
	}
}

void CopperWireEntity::performCollisions(Swan::Ctx &ctx)
{
	auto &fluids = ctx.plane.fluids();

	for (size_t i = 1; i < points_.size() - 1; ++i) {
		auto &point = points_[i];
		while (true) {
			int64_t fluidY = int64_t(floor((point.y) * Swan::FLUID_RESOLUTION));
			Swan::FluidPos fluidPos = {
				int64_t(floor(point.x * Swan::FLUID_RESOLUTION)),
				fluidY,
			};

			if (!fluids.isFluidCellSolid(fluidPos)) {
				break;
			}

			point.y = (float(fluidY) / Swan::FLUID_RESOLUTION) - 0.01;
		}
	}
}

}
