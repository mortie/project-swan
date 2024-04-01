#pragma once

#include <swan/swan.h>

namespace CoreMod {

class FallingTileEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait {
public:
	FallingTileEntity(const Swan::Context &ctx, Swan::Vec2 pos, Swan::Tile::ID tile);
	FallingTileEntity(const Swan::Context &ctx)
	{}

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

	void serialize(const Swan::Context &ctx, MsgStream::MapBuilder &w) override;
	void deserialize(const Swan::Context &ctx, MsgStream::MapParser &r) override;

private:
	static constexpr Swan::BasicPhysicsBody::Props PROPS = {
		.size = {1, 1},
		.mass = 80,
	};
	static constexpr float DESPAWN_TIME = 5 * 60;

	Swan::Tile::ID tile_ = Swan::World::INVALID_TILE_ID;
	Swan::BasicPhysicsBody physicsBody_{PROPS};
};

}
