#pragma once

#include <swan/swan.h>
#include <array>

class PlayerEntity: public Swan::PhysicsEntity, public Swan::InventoryTrait::HasInventory {
public:
	PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos);
	PlayerEntity(const Swan::Context &ctx, const PackObject &obj);

	Swan::InventoryTrait::Inventory &getInventory() override { return inventory_; }

	void draw(const Swan::Context &ctx, Swan::Win &win) override;
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

	Swan::InventoryTrait::BasicInventory inventory_;

	enum class State {
		IDLE,
		RUNNING_L,
		RUNNING_R,
		COUNT,
	};

	State state_ = State::IDLE;
	std::array<Swan::Animation, (int)State::COUNT> anims_;

	Swan::Clock jump_timer_;
	Swan::TilePos mouse_tile_;
};
