#include "CopperWireEntity.h"
#include "cygnet/Renderer.h"
#include "swan/common.h"
#include "swan/constants.h"

namespace CoreMod {

static constexpr int RESOLUTION = 2; // Segments per meter
static constexpr float SEGMENT_LENGTH = 1.0f / float(RESOLUTION);
static constexpr float SEGMENT_TENSION = 0.25;
static constexpr float SPRING_COEF = 500;
static constexpr float GRAVITY = 4;

static constexpr float TICK_HZ = 500;

void CopperWireEntity::setEndPoint(Swan::Vec2 endPoint)
{
	assert(points_.size() > 0);

	auto pointer = endPoint - points_.front();
	float length = pointer.length();

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
	timer_ += dt;
	while (timer_ > 0) {
		simulatePhysicsStep(ctx, 1.0 / TICK_HZ);
		timer_ -= 1.0 / TICK_HZ;
	}

	performCollisions(ctx);
}

void CopperWireEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	for (auto point: points_) {
		rnd.drawRect({
			.pos = point.add(-0.05, -0.05),
			.size = {0.1, 0.1},
			.fill = {0.46, 0.84, 0.17},
		});
	}
}

void CopperWireEntity::serialize(Swan::Ctx &ctx, Proto::Builder w)
{
	// TODO
}

void CopperWireEntity::deserialize(Swan::Ctx &ctx, Proto::Reader r)
{
	// TODO
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
		float leftError = (leftLength - SEGMENT_LENGTH * SEGMENT_TENSION) / 2;
		float rightError = (rightLength - SEGMENT_LENGTH * SEGMENT_TENSION) / 2;

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
