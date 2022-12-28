#pragma once

#include <swan/swan.h>

class SpiderEntity: public Swan::PhysicsEntity {
public:
	SpiderEntity(const Swan::Context &ctx, Swan::Vec2 pos):
		PhysicsEntity(pos, SIZE),
		idleAnimation_(ctx.world.getSprite("core::entity/spider-still")) {}

	SpiderEntity(const Swan::Context &ctx, const PackObject &obj) {
		deserialize(ctx, obj);
	}

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;
	void deserialize(const Swan::Context &ctx, const PackObject &obj) override;
	PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) override;

private:
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.6, 0.6);

	Swan::Animation idleAnimation_;
};
