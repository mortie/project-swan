#pragma once

#include <swan/swan.h>

namespace CoreMod {

class FallingTileEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	FallingTileEntity(const Swan::Context &ctx, Swan::Vec2 pos, Swan::Tile::ID tile);
	FallingTileEntity(const Swan::Context &ctx, const PackObject &obj);

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
	static constexpr float MASS = 80;
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(1, 1);
	static constexpr float DESPAWN_TIME = 5 * 60;
	static constexpr float BOUNCINESS = 0.6;

	Swan::Tile::ID tile_;

	Swan::BasicPhysicsBody physicsBody_{SIZE, {.mass = MASS, .bounciness = BOUNCINESS}};
};

}
