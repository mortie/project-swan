#include "traits/PhysicsTrait.h"

#include "WorldPlane.h"
#include "Win.h"

namespace Swan {

static float epsilon = 0.001;

static void collideX(
		PhysicsTrait::Physics &phys, BodyTrait::Body &body,
		WorldPlane &plane, const PhysicsTrait::PhysicsProps &props) {
	bool collided = false;

	for (int y = (int)floor(body.top() + epsilon); y <= (int)floor(body.bottom() - epsilon); ++y) {
		int lx = (int)floor(body.left() + epsilon);
		Tile &left = plane.getTile({ lx, y });
		if (left.isSolid) {
			body.pos.x = (float)lx + 1.0;
			collided = true;
			break;
		}

		int rx = (int)floor(body.right() - epsilon);
		Tile &right = plane.getTile({ rx, y });
		if (right.isSolid) {
			body.pos.x = (float)rx - body.size.x;
			collided = true;
			break;
		}
	}

	if (collided) {
		phys.vel.x *= -props.bounciness;
		if (abs(phys.vel.x) < props.mushyness)
			phys.vel.x = 0;
	}
}

static void collideY(
		PhysicsTrait::Physics &phys, BodyTrait::Body &body,
		WorldPlane &plane, const PhysicsTrait::PhysicsProps &props) {
	bool collided = false;
	phys.onGround = false;

	for (int x = (int)floor(body.left() + epsilon); x <= (int)floor(body.right() - epsilon); ++x) {
		int ty = (int)floor(body.top() + epsilon);
		Tile &top = plane.getTile({ x, ty });
		if (top.isSolid) {
			body.pos.y = (float)ty + 1.0;
			collided = true;
			break;
		}

		int by = (int)floor(body.bottom() - epsilon);
		Tile &bottom = plane.getTile({ x, by });
		if (bottom.isSolid) {
			body.pos.y = (float)by - body.size.y;
			collided = true;
			phys.onGround = true;
			break;
		}
	}

	if (collided) {
		phys.vel.y *= -props.bounciness;
		if (abs(phys.vel.y) < props.mushyness)
			phys.vel.y = 0;
	}
}

void PhysicsTrait::Physics::update(
		const Swan::Context &ctx, float dt,
		BodyTrait::Body &body, const PhysicsProps &props) {
	vel += (force / props.mass) * dt;
	force = { 0, 0 };

	Vec2 dist = vel * dt;
	Vec2 dir = dist.sign();
	Vec2 step = dir * 0.4;

	// Move in increments of at most 'step', on the X axis
	while (abs(dist.x) > abs(step.x)) {
		body.pos.x += step.x;
		collideX(*this, body, ctx.plane, props);
		dist.x -= step.x;
	}
	body.pos.x += dist.x;
	collideX(*this, body, ctx.plane, props);

	// Move in increments of at most 'step', on the Y axis
	while (abs(dist.y) > abs(step.y)) {
		body.pos.y += step.y;
		collideY(*this, body, ctx.plane, props);
		dist.y -= step.y;
	}
	body.pos.y += dist.y;
	collideY(*this, body, ctx.plane, props);
}

}
