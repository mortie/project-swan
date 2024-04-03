#pragma once

#include <swan/swan.h>

namespace CoreMod {

class PlayerEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait, public Swan::InventoryTrait {
public:
	PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos);
	PlayerEntity(const Swan::Context &ctx):
		idleAnimation_(ctx, "core::entities/player/idle", 0.2),
		runningAnimation_(ctx, "core::entities/player/running", 0),
		fallingAnimation_(ctx, "core::entities/player/falling", 0.1),
		jumpingAnimation_(ctx, "core::entities/player/jumping", 0.1),
		landingAnimation_(ctx, "core::entities/player/landing", 0.1),
		snapSound_(ctx.world.getSound("core::sounds/snap"))
	{}

	Body &get(BodyTrait::Tag) override
	{
		return physicsBody_.body;
	}

	PhysicsBody &get(PhysicsBodyTrait::Tag) override
	{
		return physicsBody_;
	}

	Inventory &get(InventoryTrait::Tag) override
	{
		return inventory_;
	}

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void ui(const Swan::Context &ctx) override;
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;

	void serialize(const Swan::Context &ctx, MsgStream::MapBuilder &w) override;
	void deserialize(const Swan::Context &ctx, MsgStream::MapParser &r) override;

private:
	enum class State {
		IDLE,
		RUNNING,
		JUMPING,
		FALLING,
		LANDING,
	};

	static constexpr Swan::BasicPhysicsBody::Props PROPS = {
		.size = {0.6, 1.9},
		.mass = 80,
		.bounciness = 0,
	};

	static constexpr int INVENTORY_SIZE = 18;
	static constexpr float SPRINT_FORCE_GROUND = 125 * PROPS.mass;
	static constexpr float MOVE_FORCE_GROUND = 75 * PROPS.mass;
	static constexpr float MOVE_FORCE_AIR = 10 * PROPS.mass;
	static constexpr float JUMP_VEL = 11;
	static constexpr float DOWN_FORCE = 20 * PROPS.mass;
	static constexpr float LADDER_CLIMB_FORCE = 70 * PROPS.mass;
	static constexpr float LADDER_MAX_VEL = 5;

	void onRightClick(const Swan::Context &ctx);
	void onLeftClick(const Swan::Context &ctx);
	void craft(const Swan::Recipe &recipe);
	void dropItem(const Swan::Context &ctx);

	State state_ = State::IDLE;
	Swan::Animation idleAnimation_;
	Swan::Animation runningAnimation_;
	Swan::Animation fallingAnimation_;
	Swan::Animation jumpingAnimation_;
	Swan::Animation landingAnimation_;
	Swan::Animation *currentAnimation_ = &idleAnimation_;

	Swan::SoundAsset *snapSound_;

	Swan::Clock jumpTimer_;
	float invincibleTimer_ = 0;
	int selectedInventorySlot_ = 0;
	int lastDirection_ = 1;
	bool sprinting_ = false;
	bool showInventory_ = false;
	float stepTimer_ = 0;
	int stepIndex_ = 0;
	float interactTimer_ = 0;

	Swan::TilePos breakPos_;
	Swan::TilePos placePos_;

	Swan::BasicInventory inventory_{INVENTORY_SIZE};
	Swan::BasicPhysicsBody physicsBody_{PROPS};
};

}
