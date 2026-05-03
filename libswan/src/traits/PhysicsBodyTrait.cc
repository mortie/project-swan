#include "traits/PhysicsBodyTrait.h"

#include <cmath>

#include "WorldPlane.h"
#include "swan/constants.h"

namespace Swan {

static float epsilon = 0.05;

static void collideX(BasicPhysicsBody &phys, WorldPlane &plane)
{
	bool collided = false;

	int64_t firstY = (int64_t)floor(phys.body.top() * FLUID_RESOLUTION + epsilon);
	int64_t lastY = (int64_t)floor(phys.body.bottom() * FLUID_RESOLUTION - epsilon);
	int64_t lx = (int64_t)floor(phys.body.left() * FLUID_RESOLUTION);
	int64_t rx = (int64_t)floor(phys.body.right() * FLUID_RESOLUTION);

	for (int64_t y = firstY; y <= lastY; ++y) {
		bool leftSolid = plane.fluids().isFluidCellSolid({lx, y});
		if (leftSolid && phys.onGround && y > lastY - phys.stepHeight) {
			phys.body.pos.y -= 1.0 / FLUID_RESOLUTION;
			phys.body.pos.x -= epsilon;
			phys.vel.x /= 1.5;
			break;
		} else if (leftSolid) {
			phys.body.pos.x = float(lx) / FLUID_RESOLUTION + 1.0 / FLUID_RESOLUTION;
			collided = true;
			break;
		}

		bool rightSolid = plane.fluids().isFluidCellSolid({rx, y});
		if (rightSolid && phys.onGround && y > lastY - phys.stepHeight) {
			phys.body.pos.y -= 1.0 / FLUID_RESOLUTION;
			phys.body.pos.x += epsilon;
			phys.vel.x /= 1.5;
			break;
		} else if (rightSolid) {
			phys.body.pos.x = float(rx) / FLUID_RESOLUTION - phys.body.size.x;
			collided = true;
			break;
		}
	}

	if (collided) {
		phys.vel.x *= -phys.bounciness;
		if (std::abs(phys.vel.x) < phys.mushyness) {
			phys.vel.x = 0;
		}
	}
}

static void collideY(BasicPhysicsBody &phys, WorldPlane &plane)
{
	bool collided = false;

	phys.onGround = false;

	int64_t firstX = (int64_t)floor(phys.body.left() * FLUID_RESOLUTION + epsilon);
	int64_t lastX = (int64_t)floor(phys.body.right() * FLUID_RESOLUTION - epsilon);
	int64_t by = (int64_t)floor(phys.body.bottom() * FLUID_RESOLUTION + 0.04);
	int64_t ty = (int64_t)floor(phys.body.top() * FLUID_RESOLUTION + 0.01);

	for (int64_t x = firstX; x <= lastX; ++x) {
		bool bottomSolid = plane.fluids().isFluidCellSolid({x, by});
		bool onGround = bottomSolid;

		// Handle platforms
		if (!onGround && phys.platformCollision && phys.vel.y >= -0.1) {
			Tile &bottom = plane.tiles().get({
				(int)floor(float(x) / FLUID_RESOLUTION),
				(int)floor(float(by) / FLUID_RESOLUTION),
			});
			onGround = bottom.isPlatform();
		}

		if (onGround) {
			phys.body.pos.y = float(by) / FLUID_RESOLUTION - phys.body.size.y;
			collided = true;
			phys.onGround = true;
			break;
		}

		bool topSolid = plane.fluids().isFluidCellSolid({x, ty});
		if (topSolid) {
			phys.body.pos.y = float(ty) / FLUID_RESOLUTION + 1.0 / FLUID_RESOLUTION;
			collided = true;
			break;
		}
	}

	if (collided) {
		phys.vel.y *= -phys.bounciness;
		if (std::abs(phys.vel.y) < phys.mushyness) {
			phys.vel.y = 0;
		}
	}
}

void BasicPhysicsBody::collideWith(BodyTrait::Body &other)
{
	if (body.bottom() < other.top() + 0.2) {
		float delta = body.bottom() - other.top();
		body.pos.y -= delta / 2;
		other.pos.y += delta / 2;
		if (vel.y > 0.01) {
			vel.y = 0.01;
		}

		onGround = true;
	}
	else if (body.top() > other.bottom() - 0.2) {
		float delta = body.top() - other.bottom();
		body.pos.y -= delta / 2;
		other.pos.y += delta / 2;
		if (vel.y < -0.01) {
			vel.y = -0.01;
		}
	}
	else if (body.right() < other.left() + 0.2) {
		float delta = body.right() - other.left();
		body.pos.x -= delta / 2;
		other.pos.x += delta / 2;
		if (vel.x > 0.01) {
			vel.x = 0.01;
		}
	}
	else if (body.left() > other.right() - 0.2) {
		float delta = body.left() - other.right();
		body.pos.x -= delta / 2;
		other.pos.x += delta / 2;
		if (vel.x < -0.01) {
			vel.x = -0.01;
		}
	}
}

void BasicPhysicsBody::collideAll(WorldPlane &plane)
{
	for (auto &c: plane.entities().getColliding(body)) {
		if (c.body.isSolid) {
			collideWith(c.body);
		}
	}
}

void BasicPhysicsBody::update(const Swan::Context &ctx, float dt)
{
	vel += (force / mass) * dt;
	force = {0, 0};

	Vec2 dist = vel * dt;
	Vec2 dir = dist.sign();
	Vec2 step = dir * 0.4;

	// Move out if we're in the center of something
	while (true) {
		auto x = (int64_t)floor(body.midX() * FLUID_RESOLUTION);
		auto y = (int64_t)floor(body.bottom() * FLUID_RESOLUTION - 0.05);
		if (ctx.plane.fluids().isFluidCellSolid({x, y})) {
			body.pos.y -= 1.0 / FLUID_RESOLUTION;
			continue;
		}

		break;
	}

	// Move in increments of at most 'step', on the Y axis
	while (std::abs(dist.y) > std::abs(step.y)) {
		body.pos.y += step.y;
		collideY(*this, ctx.plane);
		dist.y -= step.y;
	}
	body.pos.y += dist.y;
	collideY(*this, ctx.plane);

	// Move in increments of at most 'step', on the X axis
	while (std::abs(dist.x) > std::abs(step.x)) {
		body.pos.x += step.x;
		collideX(*this, ctx.plane);
		dist.x -= step.x;
	}
	body.pos.x += dist.x;
	collideX(*this, ctx.plane);
}

void BasicPhysicsBody::updateNoclip(const Swan::Context &ctx, float dt)
{
	vel += (force / mass) * dt;
	force = {0, 0};
	body.pos += vel * dt;
}

void BasicPhysicsBody::serialize(proto::BasicPhysicsBody::Builder w)
{
	auto posB = w.initPos();
	posB.setX(body.pos.x);
	posB.setY(body.pos.y);
	auto velB = w.initVel();
	velB.setX(vel.x);
	velB.setY(vel.y);
}

void BasicPhysicsBody::deserialize(proto::BasicPhysicsBody::Reader r)
{
	body.pos.x = r.getPos().getX();
	body.pos.y = r.getPos().getY();
	vel.x = r.getVel().getX();
	vel.y = r.getVel().getY();

	onGround = false;
}

}
