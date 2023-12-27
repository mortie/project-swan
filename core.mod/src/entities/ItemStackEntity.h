#pragma once

#include <swan/swan.h>

namespace CoreMod {

class ItemStackEntity final: public Swan::Entity,
		public Swan::PhysicsBodyTrait {
public:
	ItemStackEntity(const Swan::Context &ctx, Swan::Vec2 pos, const std::string &item);
	ItemStackEntity(const Swan::Context &ctx, Swan::Vec2 pos, Swan::Vec2 vel, Swan::Item *item);
	ItemStackEntity(const Swan::Context &ctx, const PackObject &obj);

	Body &get(BodyTrait::Tag) override { return physicsBody_.body; }
	PhysicsBody &get(PhysicsBodyTrait::Tag) override { return physicsBody_; }

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;
	void deserialize(const Swan::Context &ctx, const PackObject &obj) override;
	PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) override;

	Swan::Item *item() { return item_; }

	float lifetime_ = 0;

private:
	static constexpr float MASS = 80;
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.5, 0.5);
	static constexpr float DESPAWN_TIME = 5 * 60;
	static constexpr float BOUNCINESS = 0.6;

	Swan::Item *item_ = NULL;

	Swan::BasicPhysicsBody physicsBody_{SIZE, {.mass = MASS, .bounciness = BOUNCINESS}};
};

}
