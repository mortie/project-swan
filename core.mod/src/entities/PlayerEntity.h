#pragma once

#include <memory>
#include <swan/swan.h>

#include "core_mod.capnp.h"
#include "util/CraftingInventory.h"
#include "util/InteractionManager.h"

namespace CoreMod {

class PlayerEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait,
	public Swan::InventoryTrait {
public:
	using CloseInventoryCallback = void(Swan::Ctx &, Swan::EntityRef);

	PlayerEntity(Swan::Ctx &ctx);
	PlayerEntity(Swan::Ctx &ctx, Swan::Vec2 pos);

	Swan::Body &get(BodyTrait::Tag) override
	{
		return physicsBody_.body;
	}

	Swan::PhysicsBody &get(PhysicsBodyTrait::Tag) override
	{
		return physicsBody_;
	}

	Swan::Inventory &get(InventoryTrait::Tag) override
	{
		return inventory_;
	}

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void update(Swan::Ctx &ctx, float dt) override;
	void tick(Swan::Ctx &ctx, float dt) override;
	void drawDebug(Swan::Ctx &ctx) override;

	void serialize(Swan::Ctx &ctx, capnp::MessageBuilder &mb) override;
	void deserialize(Swan::Ctx &ctx, capnp::MessageReader &mr) override;

	bool askToOpenInventory(Swan::EntityRef ent, CloseInventoryCallback cb);
	void askToCloseInventory(Swan::Ctx &ctx, Swan::EntityRef ent);
	Swan::EntityRef currentInventoryEntity() { return auxInventoryEntity_; }

	void registerInteractionManager(std::unique_ptr<InteractionManager> manager)
	{ interactionManager_ = std::move(manager); }
	void clearInteractionManager()
	{ interactionManager_.reset(); }

	void hurt(Swan::Ctx &ctx, int n);
	bool heal(Swan::Ctx &, int n);

private:
	struct UI {
		int selectedInventorySlot = 0;
		int hoveredInventorySlot = -1;
		bool showInventory = false;
		Cygnet::Rect hotbarRect;
		Cygnet::Rect inventoryRect;

		int hoveredAuxInventorySlot = -1;
		Cygnet::Rect auxInventoryRect;
	};

	struct HeldLight {
		Swan::TilePos pos;
		float level;

		friend bool operator==(const HeldLight &a, const HeldLight &b)
		{
			return a.pos == b.pos && a.level == b.level;
		}
	};

	enum class State {
		IDLE,
		RUNNING,
		JUMPING,
		FALLING,
		LANDING,
	};

	enum class Vit {
		OK,
		WINDED,
		LETHARGIC,
	};

	Swan::Animation idleAnimation();
	Swan::Animation runningAnimation();
	Swan::Animation fallingAnimation();
	Swan::Animation jumpingAnimation();
	Swan::Animation landingAnimation();

	void onRightClick(Swan::Ctx &ctx, Swan::Vec2 lookPos);
	void onLeftClick(Swan::Ctx &ctx);
	void dropItem(Swan::Ctx &ctx);

	void handlePhysics(Swan::Ctx &ctx, float dt);
	void handleInventoryClick(Swan::Ctx &ctx);
	void handleInventorySelection(Swan::Ctx &ctx);
	void handleInventoryHover(Swan::Ctx &ctx);

	void drawInventory(Swan::Ctx &ctx, Cygnet::Renderer &rnd);
	void drawConsole(Swan::Ctx &ctx);

	float computeAirTemperature(Swan::Ctx &ctx);

	State state_ = State::IDLE;
	Vit vit_ = Vit::OK;
	Swan::Animation currentAnimation_ = idleAnimation();

	UI ui_;

	Swan::Inventory *auxInventory_ = nullptr;
	Swan::EntityRef auxInventoryEntity_;
	CloseInventoryCallback *closeInventoryCallback_ = nullptr;

	Swan::Clock jumpTimer_;
	float invincibleTimer_ = 0;
	int lastDirection_ = 1;
	Swan::Vec2 lookVector_;
	bool sprinting_ = false;
	bool inFluid_ = false;
	Cygnet::Color fluidColor_;
	float stepTimer_ = 0;
	int stepIndex_ = 0;
	float interactTimer_ = 0;
	Swan::TilePos spawnPoint_;
	int health_ = 0;
	float invulnerable_ = 0;
	float blackout_ = 0;
	float oxygen_ = 0;
	float temperature_ = 0;

	std::unique_ptr<InteractionManager> interactionManager_;

	int teleState_ = 0;
	float teleportTimer_ = 0;

	bool inLadder_ = false;
	bool inWorkbench_ = false;

	float platformCollisionTimer_ = 0;

	float gamma_ = 1;

	Swan::TilePos breakPos_;
	Swan::TilePos placePos_;
	bool mouseMode_ = false;

	Swan::ItemStack heldStack_;
	std::optional<HeldLight> heldLight_;

	Swan::BasicInventory inventory_;
	CraftingInventory craftingInventory_;
	Swan::BasicPhysicsBody physicsBody_;

	std::string consoleOutput_;
	std::string consoleInput_;
	bool consoleVisible_ = false;
	bool consoleFirstFrame_ = false;
};

}
