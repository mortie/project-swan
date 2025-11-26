#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"
#include "util/CraftingInventory.h"

namespace CoreMod {

class PlayerEntity final: public Swan::Entity,
	public Swan::PhysicsBodyTrait, public Swan::InventoryTrait {
public:
	using Proto = proto::PlayerEntity;

	using CloseInventoryCallback = void(Swan::Ctx &, Swan::EntityRef);

	PlayerEntity(Swan::Ctx &ctx);
	PlayerEntity(Swan::Ctx &ctx, Swan::Vec2 pos);

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

	void draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd) override;
	void update(Swan::Ctx &ctx, float dt) override;
	void tick(Swan::Ctx &ctx, float dt) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

	bool askToOpenInventory(Swan::EntityRef ent, CloseInventoryCallback cb);
	void askToCloseInventory(Swan::Ctx &ctx, Swan::EntityRef ent);

	void hurt(Swan::Ctx &ctx, int n);
	bool heal(Swan::Ctx &, int n);

private:
	struct Sounds {
		Sounds(Swan::Ctx &ctx):
			snap(ctx.world.getSound("core::misc/snap")),
			splash(ctx.world.getSound("core::misc/splash")),
			shortSplash(ctx.world.getSound("core::misc/splash-short")),
			teleport(ctx.world.getSound("core::misc/teleport")),
			inventoryOpen(ctx.world.getSound("core::ui/inventory-open")),
			inventoryClose(ctx.world.getSound("core::ui/inventory-close")),
			crafting(ctx.world.getSound("core::ui/crafting"))
		{}

		Swan::SoundAsset *snap;
		Swan::SoundAsset *splash;
		Swan::SoundAsset *shortSplash;
		Swan::SoundAsset *teleport;
		Swan::SoundAsset *inventoryOpen;
		Swan::SoundAsset *inventoryClose;
		Swan::SoundAsset *crafting;
	};

	struct Sprites {
		Sprites(Swan::Ctx &ctx):
			idle(ctx, "core::entities/player/idle", 0.2),
			running(ctx, "core::entities/player/running", 0),
			falling(ctx, "core::entities/player/falling", 0.1),
			jumping(ctx, "core::entities/player/jumping", 0.1),
			landing(ctx, "core::entities/player/landing", 0.1),
			emptyHeart(ctx.world.getSprite("core::misc/empty-heart")),
			heart(ctx.world.getSprite("core::misc/heart")),
			bubble(ctx.world.getSprite("core::misc/bubble"))
		{}

		Swan::Animation idle;
		Swan::Animation running;
		Swan::Animation falling;
		Swan::Animation jumping;
		Swan::Animation landing;
		Cygnet::RenderSprite emptyHeart;
		Cygnet::RenderSprite heart;
		Cygnet::RenderSprite bubble;
	};

	struct Actions {
		Actions(Swan::Ctx &ctx):
			cheatHeal(ctx.game.action("core::cheat-heal")),
			cheatHurt(ctx.game.action("core::cheat-hurt")),
			cheatTickWorld(ctx.game.action("core::cheat-tick-world")),
			cheatGrabItem(ctx.game.action("core::cheat-grab-item")),
			guiShowInventory(ctx.game.action("core::gui-show-inventory")),
			guiShowCrafting(ctx.game.action("core::gui-show-crafting")),
			guiClick(ctx.game.action("core::gui-click")),
			guiModifier(ctx.game.action("core::gui-modifier")),
			guiLeft(ctx.game.action("core::gui-left")),
			guiRight(ctx.game.action("core::gui-right")),
			guiUp(ctx.game.action("core::gui-up")),
			guiDown(ctx.game.action("core::gui-down")),
			returnHome(ctx.game.action("core::return-home")),
			breakTile(ctx.game.action("core::break-tile")),
			activate(ctx.game.action("core::activate")),
			dropItem(ctx.game.action("core::drop-item")),
			selectItem(ctx.game.action("core::select-item")),
			sprint(ctx.game.action("core::sprint")),
			jump(ctx.game.action("core::jump")),
			moveX(ctx.game.action("core::move-x")),
			moveY(ctx.game.action("core::move-y")),
			selectX(ctx.game.action("core::select-x")),
			selectY(ctx.game.action("core::select-y")),
			slot0(ctx.game.action("core::select-slot-0")),
			slot1(ctx.game.action("core::select-slot-1")),
			slot2(ctx.game.action("core::select-slot-2")),
			slot3(ctx.game.action("core::select-slot-3")),
			slot4(ctx.game.action("core::select-slot-4")),
			slot5(ctx.game.action("core::select-slot-5")),
			slot6(ctx.game.action("core::select-slot-6")),
			slot7(ctx.game.action("core::select-slot-7")),
			slot8(ctx.game.action("core::select-slot-8")),
			slot9(ctx.game.action("core::select-slot-9"))
		{}

		Swan::Action *cheatHeal;
		Swan::Action *cheatHurt;
		Swan::Action *cheatTickWorld;
		Swan::Action *cheatGrabItem;

		Swan::Action *guiShowInventory;
		Swan::Action *guiShowCrafting;
		Swan::Action *guiClick;
		Swan::Action *guiModifier;
		Swan::Action *guiLeft;
		Swan::Action *guiRight;
		Swan::Action *guiUp;
		Swan::Action *guiDown;

		Swan::Action *returnHome;
		Swan::Action *breakTile;
		Swan::Action *activate;
		Swan::Action *dropItem;
		Swan::Action *selectItem;
		Swan::Action *sprint;
		Swan::Action *jump;
		Swan::Action *moveX;
		Swan::Action *moveY;
		Swan::Action *selectX;
		Swan::Action *selectY;

		Swan::Action *slot0;
		Swan::Action *slot1;
		Swan::Action *slot2;
		Swan::Action *slot3;
		Swan::Action *slot4;
		Swan::Action *slot5;
		Swan::Action *slot6;
		Swan::Action *slot7;
		Swan::Action *slot8;
		Swan::Action *slot9;
	};

	struct UI {
		int selectedInventorySlot = 0;
		int hoveredInventorySlot = -1;
		bool showInventory = false;
		Cygnet::Renderer::Rect hotbarRect;
		Cygnet::Renderer::Rect inventoryRect;

		int hoveredAuxInventorySlot = -1;
		Cygnet::Renderer::Rect auxInventoryRect;
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

	void onRightClick(Swan::Ctx &ctx);
	void onLeftClick(Swan::Ctx &ctx);
	void dropItem(Swan::Ctx &ctx);

	void handlePhysics(Swan::Ctx &ctx, float dt);
	void handleInventoryClick(Swan::Ctx &ctx);
	void handleInventorySelection(Swan::Ctx &ctx);
	void handleInventoryHover(Swan::Ctx &ctx);

	State state_ = State::IDLE;
	Vit vit_ = Vit::OK;
	Sprites sprites_;
	Actions actions_;
	Swan::Animation *currentAnimation_ = &sprites_.idle;

	Sounds sounds_;
	Cygnet::RenderSprite inventorySprite_;
	Cygnet::RenderSprite selectedSlotSprite_;

	UI ui_;

	Swan::InventoryTrait::Inventory *auxInventory_ = nullptr;
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

	int teleState_ = 0;
	float teleportTimer_ = 0;

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
};

}
