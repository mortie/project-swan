#pragma once

#include <swan/swan.h>

namespace CoreMod {

class SpiderEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait, public Swan::ContactDamageTrait {
public:
	SpiderEntity(const Swan::Context &ctx, Swan::Vec2 pos);
	SpiderEntity(const Swan::Context &ctx):
		idleAnimation_(ctx, "core::entities/spider/idle", 0.8)
	{}

	Body &get(BodyTrait::Tag) override
	{
		return physicsBody_.body;
	}

	PhysicsBody &get(PhysicsBodyTrait::Tag) override
	{
		return physicsBody_;
	}

	Damage &get(ContactDamageTrait::Tag) override
	{
		return damage_;
	}

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;

	void serialize(const Swan::Context &ctx, nbon::ObjectWriter w) override;
	void deserialize(const Swan::Context &ctx, nbon::ObjectReader r) override;

private:
	static constexpr Swan::BasicPhysicsBody::Props PROPS = {
		.size = {1, 0.65},
		.mass = 30,
	};
	static constexpr float MOVE_FORCE = 50 * PROPS.mass;
	static constexpr float JUMP_VEL = 9;

	Swan::Animation idleAnimation_;

	float jumpTimer_ = 0;
	Body *target_ = nullptr;

	Swan::BasicPhysicsBody physicsBody_{PROPS};
	Damage damage_{};
};

}
