#include "traits/PhysicsBodyTrait.h"

#include <cmath>

#include "WorldPlane.h"
#include "util.h"

namespace Swan {

static float epsilon = 0.05;

static void collideX(BasicPhysicsBody &phys, WorldPlane &plane)
{
	bool collided = false;

	int firstY = (int)floor(phys.body.top() + epsilon);
	int lastY = (int)floor(phys.body.bottom() - epsilon);

	for (int y = firstY; y <= lastY; ++y) {
		int lx = (int)floor(phys.body.left());
		Tile &left = plane.getTile({lx, y});
		if (left.isSolid) {
			phys.body.pos.x = (float)lx + 1.0;
			collided = true;
			break;
		}

		int rx = (int)floor(phys.body.right());
		Tile &right = plane.getTile({rx, y});
		if (right.isSolid) {
			phys.body.pos.x = (float)rx - phys.body.size.x;
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

	int firstX = (int)floor(phys.body.left() + epsilon);
	int lastX = (int)floor(phys.body.right() - epsilon);
	for (int x = firstX; x <= lastX; ++x) {
		int by = (int)floor(phys.body.bottom());
		Tile &bottom = plane.getTile({x, by});
		if (bottom.isSolid) {
			phys.body.pos.y = (float)by - phys.body.size.y;
			collided = true;
			phys.onGround = true;
			break;
		}

		int ty = (int)floor(phys.body.top());
		Tile &top = plane.getTile({x, ty});
		if (top.isSolid) {
			phys.body.pos.y = (float)ty + 1.0;
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

void BasicPhysicsBody::collideWith(const BodyTrait::Body &other)
{
	auto dist = Swan::max(0.5,
		std::abs(body.bottom() - other.top()),
		std::abs(body.top() - other.bottom()),
		std::abs(body.left() - other.right()),
		std::abs(body.right() - other.left()));

	auto direction = (body.center() - other.center()).norm();

	applyForce(direction * dist * 10000);
}

void BasicPhysicsBody::collideAll(WorldPlane &plane)
{
	for (auto &c: plane.getCollidingEntities(body)) {
		collideWith(c.body);
	}
}

void BasicPhysicsBody::update(const Swan::Context &ctx, float dt)
{
	vel += (force / mass) * dt;
	force = {0, 0};

	Vec2 dist = vel * dt;
	Vec2 dir = dist.sign();
	Vec2 step = dir * 0.4;

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

void BasicPhysicsBody::serialize(MsgStream::Serializer &w)
{
	auto arr = w.beginArray(4);

	arr.writeFloat32(body.pos.x);
	arr.writeFloat32(body.pos.y);
	arr.writeFloat32(vel.x);
	arr.writeFloat32(vel.y);
	w.endArray(arr);
}

void BasicPhysicsBody::deserialize(MsgStream::Parser &r)
{
	auto arr = r.nextArray();

	body.pos.x = arr.nextFloat32();
	body.pos.y = arr.nextFloat32();
	vel.x = arr.nextFloat32();
	vel.y = arr.nextFloat32();
	onGround = false;
}

}
