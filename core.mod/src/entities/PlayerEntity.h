#pragma once

#include <swan/swan.h>

namespace CoreMod {

class PlayerEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait, public Swan::InventoryTrait {
public:
	PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos);
	PlayerEntity(const Swan::Context &ctx, const PackObject &obj);

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
	void deserialize(const Swan::Context &ctx, const PackObject &obj) override;
	PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) override;

private:
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.6, 1.9);
	static constexpr float MASS = 80;
	static constexpr int INVENTORY_SIZE = 18;
	static constexpr float SPRINT_FORCE_GROUND = 125 * MASS;
	static constexpr float MOVE_FORCE_GROUND = 75 * MASS;
	static constexpr float MOVE_FORCE_AIR = 10 * MASS;
	static constexpr float JUMP_VEL = 11;
	static constexpr float DOWN_FORCE = 20 * MASS;
	static constexpr float LADDER_CLIMB_FORCE = 70 * MASS;
	static constexpr float LADDER_MAX_VEL = 5;

	enum class State {
		IDLE,
		RUNNING,
		JUMPING,
		FALLING,
		LANDING,
	};

	void placeTile(const Swan::Context &ctx);
	void craft(const Swan::Recipe &recipe);
	void dropItem(const Swan::Context &ctx);

	PlayerEntity(const Swan::Context &ctx):
		idleAnimation_(ctx.world.getSprite("core::entities/player/idle"), 0.2),
		runningAnimation_(ctx.world.getSprite("core::entities/player/running"), 0),
		fallingAnimation_(ctx.world.getSprite("core::entities/player/falling"), 0.1),
		jumpingAnimation_(ctx.world.getSprite("core::entities/player/jumping"), 0.1),
		landingAnimation_(ctx.world.getSprite("core::entities/player/landing"), 0.1)
	{}

	State state_ = State::IDLE;
	Swan::Animation idleAnimation_;
	Swan::Animation runningAnimation_;
	Swan::Animation fallingAnimation_;
	Swan::Animation jumpingAnimation_;
	Swan::Animation landingAnimation_;
	Swan::Animation *currentAnimation_ = &idleAnimation_;

	Swan::Clock jumpTimer_;
	float invincibleTimer_ = 0;
	Swan::TilePos mouseTile_;
	int selectedInventorySlot_ = 0;
	int lastDirection_ = 1;
	bool sprinting_ = false;
	bool showInventory_ = false;

	Swan::BasicInventory inventory_{INVENTORY_SIZE};
	Swan::BasicPhysicsBody physicsBody_{SIZE, {.mass = MASS}};
};

}
