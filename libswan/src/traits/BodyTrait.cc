#include "traits/BodyTrait.h"

#include <math.h>
#include <array>
#include <algorithm>

#include "WorldPlane.h"
#include "Win.h"

namespace Swan {
namespace BodyTrait {

static float epsilon = 0.0001;

void PhysicsBody::friction(Vec2 coef) {
	force_ += -vel_ * coef;
}

void PhysicsBody::gravity(Vec2 g) {
	force_ += g * mass_;
}

void PhysicsBody::collideX(WorldPlane &plane) {
	auto bounds = getBounds();
	bool collided = false;

	for (int y = (int)floor(bounds.top() + epsilon); y <= (int)floor(bounds.bottom() - epsilon); ++y) {
		int lx = (int)floor(bounds.left() + epsilon);
		Tile &left = plane.getTile({ lx, y });
		if (left.is_solid_) {
			bounds.pos.x = (float)lx + 1.0;
			collided = true;
			break;
		}

		int rx = (int)floor(bounds.right() - epsilon);
		Tile &right = plane.getTile({ rx, y });
		if (right.is_solid_) {
			bounds.pos.x = (float)rx - bounds.size.x;
			collided = true;
			break;
		}
	}

	if (collided) {
		pos_.x = bounds.pos.x;

		vel_.x *= -bounciness_;
		if (abs(vel_.x) < mushyness_)
			vel_.x = 0;
	}
}

void PhysicsBody::collideY(WorldPlane &plane) {
	auto bounds = getBounds();
	bool collided = false;
	on_ground_ = false;

	for (int x = (int)floor(bounds.left() + epsilon); x <= (int)floor(bounds.right() - epsilon); ++x) {
		int ty = (int)floor(bounds.top() + epsilon);
		Tile &top = plane.getTile({ x, ty });
		if (top.is_solid_) {
			bounds.pos.y = (float)ty + 1.0;
			collided = true;
			break;
		}

		int by = (int)floor(bounds.bottom() - epsilon);
		Tile &bottom = plane.getTile({ x, by });
		if (bottom.is_solid_) {
			bounds.pos.y = (float)by - bounds.size.y;
			collided = true;
			on_ground_ = true;
			break;
		}
	}

	if (collided) {
		pos_.y = bounds.pos.y;

		vel_.y *= -bounciness_;
		if (abs(vel_.y) < mushyness_)
			vel_.y = 0;
	}
}

void PhysicsBody::outline(Win &win) {
	win.drawRect(pos_, size_);
}

void PhysicsBody::update(const Swan::Context &ctx, float dt) {
	vel_ += (force_ / mass_) * dt;
	force_ = { 0, 0 };

	Vec2 dist = vel_ * dt;
	Vec2 dir = dist.sign();
	Vec2 step = dir * 0.4;

	// Move in increments of at most 'step', on the X axis
	while (abs(dist.x) > abs(step.x)) {
		pos_.x += step.x;
		collideX(ctx.plane);
		dist.x -= step.x;
	}
	pos_.x += dist.x;
	collideX(ctx.plane);

	// Move in increments of at most 'step', on the Y axis
	while (abs(dist.y) > abs(step.y)) {
		pos_.y += step.y;
		collideY(ctx.plane);
		dist.y -= step.y;
	}
	pos_.y += dist.y;
	collideY(ctx.plane);
}

void PhysicsBody::updateWithoutCollision(float dt) {
	vel_ += (force_ / mass_) * dt;
	pos_ += vel_ * dt;
	force_ = { 0, 0 };
}

}
}
