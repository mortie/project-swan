#pragma once

#include <swan/swan.h>
#include <array>

class PlayerEntity: public Swan::PhysicsEntity, public Swan::InventoryTrait {
public:
	PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos);
	PlayerEntity(const Swan::Context &ctx, const PackObject &obj);

	using PhysicsEntity::get;
	Inventory &get(InventoryTrait::Tag) override { return inventory_; }

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;
	void deserialize(const Swan::Context &ctx, const PackObject &obj) override;
	PackObject serialize(const Swan::Context &ctx, msgpack::zone &zone) override;

private:
	static constexpr int INVENTORY_SIZE = 18;
	static constexpr float MASS = 80;
	static constexpr float MOVE_FORCE = 34 * MASS;
	static constexpr float JUMP_VEL = 11;
	static constexpr float DOWN_FORCE = 20 * MASS;
	static constexpr Swan::Vec2 SIZE = Swan::Vec2(0.6, 1.9);
	static constexpr float LIGHT_LEVEL = 0.2;

	enum class State {
		IDLE,
		RUNNING_L,
		RUNNING_R,
		COUNT,
	};

	PlayerEntity(const Swan::Context &ctx):
		PhysicsEntity(SIZE),
		anims_{
			Swan::Animation(ctx.world.getSprite("core::entity/player-still"), 0.8),
			Swan::Animation(
				ctx.world.getSprite("core::entity/player-running"),
				1, Cygnet::Mat3gf{}.scale({-1, 1})),
			Swan::Animation(ctx.world.getSprite("core::entity/player-running"), 1),
		} {}

	State state_ = State::IDLE;
	std::array<Swan::Animation, (int)State::COUNT> anims_;

	Swan::Clock jumpTimer_;
	Swan::Clock placeTimer_;
	Swan::TilePos mouseTile_;

	Swan::TilePos lightTile_;
	bool placedLight_ = false;

	BasicInventory inventory_{INVENTORY_SIZE};
};
