#pragma once

#include <swan/swan.h>

class SpiderEntity final: public Swan::PhysicsEntity, public Swan::ContactDamageTrait {
public:
	SpiderEntity(const Swan::Context &ctx, Swan::Vec2 pos);
	SpiderEntity(const Swan::Context &ctx, const PackObject &obj);

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;
	void deserialize(const Swan::Context &ctx, const PackObject &obj) override;
	PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) override;

	using PhysicsEntity::get;
	Damage &get(ContactDamageTrait::Tag) override { return damage_; }

private:
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(1, 0.65);
	static constexpr float MASS = 30;
	static constexpr float MOVE_FORCE = 50 * MASS;
	static constexpr float JUMP_VEL = 9;

	SpiderEntity(const Swan::Context &ctx):
		PhysicsEntity(SIZE),
		idleAnimation_(ctx.world.getSprite("core::entity/spider-idle"), 0.8) {}

	Swan::Animation idleAnimation_;

	float jumpTimer_ = 0;

	Body *target_ = nullptr;
	Damage damage_{};
};
