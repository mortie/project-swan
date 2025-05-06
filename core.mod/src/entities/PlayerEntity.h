#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class PlayerEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait, public Swan::InventoryTrait {
public:
	using Proto = proto::PlayerEntity;

	PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos);
	PlayerEntity(const Swan::Context &ctx):
		animations_(ctx),
		sounds_(ctx),
		inventorySprite_(ctx.world.getSprite("core::ui/inventory")),
		selectedSlotSprite_(ctx.world.getSprite("core::ui/selected-slot"))
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
	void update(const Swan::Context &ctx, float dt) override;
	void tick(const Swan::Context &ctx, float dt) override;

	void serialize(const Swan::Context &ctx, Proto::Builder w);
	void deserialize(const Swan::Context &ctx, Proto::Reader r);

private:
	struct Sounds {
		Sounds(const Swan::Context &ctx):
			snap(ctx.world.getSound("core::sounds/ui/snap")),
			splash(ctx.world.getSound("core::sounds/ui/splash")),
			shortSplash(ctx.world.getSound("core::sounds/ui/splash-short")),
			inventoryOpen(ctx.world.getSound("core::sounds/ui/inventory-open")),
			inventoryClose(ctx.world.getSound("core::sounds/ui/inventory-close")),
			crafting(ctx.world.getSound("core::sounds/ui/crafting"))
		{}

		Swan::SoundAsset *snap;
		Swan::SoundAsset *splash;
		Swan::SoundAsset *shortSplash;
		Swan::SoundAsset *inventoryOpen;
		Swan::SoundAsset *inventoryClose;
		Swan::SoundAsset *crafting;
	};

	struct Animations {
		Animations(const Swan::Context &ctx):
			idle(ctx, "core::entities/player/idle", 0.2),
			running(ctx, "core::entities/player/running", 0),
			falling(ctx, "core::entities/player/falling", 0.1),
			jumping(ctx, "core::entities/player/jumping", 0.1),
			landing(ctx, "core::entities/player/landing", 0.1)
		{}

		Swan::Animation idle;
		Swan::Animation running;
		Swan::Animation falling;
		Swan::Animation jumping;
		Swan::Animation landing;
	};

	enum class State {
		IDLE,
		RUNNING,
		JUMPING,
		FALLING,
		LANDING,
	};

	static constexpr Swan::BasicPhysicsBody::Props PROPS = {
		.size = {0.6, 1.5},
		.mass = 80,
		.bounciness = 0,
	};

	static constexpr int INVENTORY_SIZE = 40;
	static constexpr float SPRINT_FORCE_GROUND = 125 * PROPS.mass;
	static constexpr float MOVE_FORCE_GROUND = 75 * PROPS.mass;
	static constexpr float MOVE_FORCE_AIR = 10 * PROPS.mass;
	static constexpr float JUMP_VEL = 12.5;
	static constexpr float DOWN_FORCE = 30 * PROPS.mass;
	static constexpr float LADDER_CLIMB_FORCE = 70 * PROPS.mass;
	static constexpr float LADDER_MAX_VEL = 5;
	static constexpr float SWIM_FORCE_UP = 12 * PROPS.mass;
	static constexpr float SWIM_FORCE_DOWN = 12 * PROPS.mass;

	void onRightClick(const Swan::Context &ctx);
	void onLeftClick(const Swan::Context &ctx);
	void craft(const Swan::Context &ctx, const Swan::Recipe &recipe);
	void dropItem(const Swan::Context &ctx);

	bool handleInventoryClick(const Swan::Context &ctx);
	void handleInventorySelection(const Swan::Context &ctx);

	State state_ = State::IDLE;
	Animations animations_;
	Swan::Animation *currentAnimation_ = &animations_.idle;

	Sounds sounds_;
	Cygnet::RenderSprite inventorySprite_;
	Cygnet::RenderSprite selectedSlotSprite_;

	Swan::Clock jumpTimer_;
	float invincibleTimer_ = 0;
	int selectedInventorySlot_ = 0;
	int lastDirection_ = 1;
	bool sprinting_ = false;
	bool showInventory_ = false;
	bool inFluid_ = false;
	Cygnet::Color fluidColor_;
	float stepTimer_ = 0;
	int stepIndex_ = 0;
	float interactTimer_ = 0;
	Swan::TilePos spawnPoint_;

	float gamma_ = 1;

	Swan::TilePos breakPos_;
	Swan::TilePos placePos_;

	Swan::ItemStack heldStack_;

	Swan::BasicInventory inventory_{INVENTORY_SIZE};
	Swan::BasicPhysicsBody physicsBody_{PROPS};
};

}
