#pragma once

#include <swan/swan.h>

namespace CoreMod {

class DynamiteEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	DynamiteEntity(
		const Swan::Context &ctx, Swan::Vec2 pos, Swan::Vec2 vel = {0, 0});
	DynamiteEntity(const Swan::Context &ctx, const PackObject &obj);

	Body &get(BodyTrait::Tag) override
	{
		return physicsBody_.body;
	}

	PhysicsBody &get(PhysicsBodyTrait::Tag) override
	{
		return physicsBody_;
	}

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void update(const Swan::Context &ctx, float dt) override;

	void deserialize(const Swan::Context &ctx, const PackObject &obj) override;
	PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) override;

private:
	static constexpr Swan::BasicPhysicsBody::Props PROPS = {
		.mass = 20,
		.size = {0.8, 0.2},
	};
	static constexpr float FUSE_TIME = 5;

	Swan::Tile::ID tile_;
	float fuse_ = FUSE_TIME;
	Swan::BasicPhysicsBody physicsBody_{PROPS};
};

}
