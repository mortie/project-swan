#include "traits/PhysicsBodyTrait.h"

#include <cmath>

#include "WorldPlane.h"

namespace Swan {

static float epsilon = 0.05;

static void collideX(BasicPhysicsBody &phys, WorldPlane &plane)
{
	bool collided = false;

	int firstY = (int)floor(phys.body.top() + epsilon);
	int lastY = (int)floor(phys.body.bottom() - epsilon);

	for (int y = firstY; y <= lastY; ++y) {
		int lx = (int)floor(phys.body.left());
		Tile &left = plane.tiles().get({lx, y});
		if (left.isSolid) {
			phys.body.pos.x = (float)lx + 1.0;
			collided = true;
			break;
		}

		int rx = (int)floor(phys.body.right());
		Tile &right = plane.tiles().get({rx, y});
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
		int by = (int)floor(phys.body.bottom() + 0.04);
		Tile &bottom = plane.tiles().get({x, by});
		if (bottom.isSolid) {
			phys.body.pos.y = (float)by - phys.body.size.y;
			collided = true;
			phys.onGround = true;
			break;
		}

		int ty = (int)floor(phys.body.top() + 0.01);
		Tile &top = plane.tiles().get({x, ty});
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
