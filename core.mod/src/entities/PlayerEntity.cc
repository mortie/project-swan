#include "PlayerEntity.h"

#include <algorithm>
#include <imgui/imgui.h>
#include <string>

#include "ItemStackEntity.h"
#include "world/util.h"
#include "world/ladder.h"

namespace CoreMod {

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
static constexpr int MAX_HEALTH = 10;
static constexpr float BLACKOUT_TIME = 5;
static constexpr float MAX_OXYGEN = 12;

PlayerEntity::PlayerEntity(Swan::Ctx &ctx):
	sprites_(ctx),
	sounds_(ctx),
	inventorySprite_(ctx.world.getSprite("core::ui/inventory")),
	selectedSlotSprite_(ctx.world.getSprite("core::ui/selected-slot")),
	health_(MAX_HEALTH),
	oxygen_(MAX_OXYGEN),
	inventory_(INVENTORY_SIZE),
	craftingInventory_(ctx.plane.entities().current()),
	physicsBody_(PROPS)
{}

PlayerEntity::PlayerEntity(Swan::Ctx &ctx, Swan::Vec2 pos):
	PlayerEntity(ctx)
{
	physicsBody_.body.pos = pos;
	spawnPoint_ = pos.as<int>();
}

void PlayerEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	if (invulnerable_ > 0) {
		rnd.setGamma(gamma_ + invulnerable_ * 3);
	} else if (vit_ == Vit::LETHARGIC) {
		rnd.setGamma(gamma_ + 0.5);
	} else {
		rnd.setGamma(gamma_);
	}

	float blackoutAlpha = 0;
	if (oxygen_ < 4) {
		blackoutAlpha = (4 - oxygen_) / 3.5;
	}

	if (blackout_ > 0) {
		float alpha = 1.0;
		if (blackout_ < 0.3) {
			alpha = blackout_ / 0.3;
		}
		else if (blackout_ > BLACKOUT_TIME - 0.5) {
			alpha = 0;
		}
		else if (blackout_ > BLACKOUT_TIME - 2.0) {
			alpha = (BLACKOUT_TIME - blackout_ - 0.5) / 1.5;
		}

		if (alpha > blackoutAlpha) {
			blackoutAlpha = alpha;
		}
	}

	if (blackoutAlpha > 0) {
		rnd.drawUIRect(Cygnet::RenderLayer::FOREGROUND, {
			.pos = {},
			.size = {100, 100},
			.fill = {0, 0, 0, blackoutAlpha},
		});
	}

	Cygnet::Mat3gf mat;

	// Currently, there is no sprite for running left.
	// Running left is just running right but flipped.
	if (lastDirection_ < 0) {
		mat.translate({-0.9, 0}).scale({-1, 1}).translate({0.9, 0});
	}

	// Teleportation animation..
	if (teleState_ != 0) {
		float frac = teleportTimer_ / 0.2;
		mat.scale({frac, 1});
		mat.translate({(1.0f - frac) * 0.9f, 0});
	}

	// The running animation dips into the ground a bit
	if (state_ == State::RUNNING) {
		mat.translate({0, 3.5 / 32.0});
	}

	// Position
	mat.translate(physicsBody_.body.pos - Swan::Vec2{0.6, 0.5});

	currentAnimation_->draw(rnd, mat);

	rnd.drawRect({Swan::Vec2(placePos_).add(0.1, 0.1), {0.8, 0.8}});
	rnd.drawRect({breakPos_, {1, 1}});

	// Draw health
	rnd.uiView({}, [&] {
		for (int i = 0; i < std::max(MAX_HEALTH, health_); ++i) {
			Cygnet::RenderSprite *sprite;
			if (i >= health_) {
				sprite = &sprites_.emptyHeart;
			}
			else {
				sprite = &sprites_.heart;
			}

			float y = 0.25;
			if (vit_ != Vit::OK && i % 2 == 1) {
				y += 0.05;
			}

			rnd.drawUISprite({
				.transform = Cygnet::Mat3gf{}
					.translate({(i / 3.5f) + 0.25f, y}),
				.sprite = *sprite,
			});
		}
	}, Cygnet::Anchor::TOP_LEFT);

	// Draw oxygen
	rnd.uiView({}, [&] {
		if (oxygen_ >= 11.5) {
			return;
		}

		for (int i = 0; i < 10; ++i) {
			if (oxygen_ < (i + 1.5)) {
				break;
			}

			float y = 0.25 + (1.0 / 3.5);

			rnd.drawUISprite({
				.transform = Cygnet::Mat3gf{}
					.translate({(i / 3.5f) + 0.25f, y}),
				.sprite = sprites_.bubble,
			});
		}
	}, Cygnet::Anchor::TOP_LEFT);

	// Draw hotbar
	ui_.hotbarRect = rnd.uiView({
		.size = {12, 3},
	}, [&] {
		// Hotbar content
		Swan::UI::inventory(
			ctx, rnd, {10, 1}, inventorySprite_, inventory_.content_,
			ui_.hoveredInventorySlot);

		// Selection
		if (ui_.selectedInventorySlot < 10) {
			rnd.drawUISprite({
				.transform = Cygnet::Mat3gf{}.translate(
					{float(ui_.selectedInventorySlot), 0}),
				.sprite = selectedSlotSprite_,
			}, Cygnet::Anchor::TOP_LEFT);
		}
	}, Cygnet::Anchor::BOTTOM);

	if (!heldStack_.empty()) {
		rnd.drawUITile(Cygnet::RenderLayer::FOREGROUND, {
			.transform = Cygnet::Mat3gf{}
				.scale({1.5, 1.5})
				.translate({ctx.game.getMouseUIPos()})
				.translate({-0.25, -0.5}),
			.id = heldStack_.item()->id,
		});

		rnd.drawUIText(Cygnet::RenderLayer::FOREGROUND, {
			.textCache = ctx.game.smallFont_,
			.transform = Cygnet::Mat3gf{}
				.scale({0.7, 0.7})
				.translate({ctx.game.getMouseUIPos()})
				.translate({0.1, 0.8}),
			.text = Swan::strify(heldStack_.count()),
		});
	}

	// Do we have another inventory?
	if (!auxInventoryEntity_.isNil()) {
		auxInventory_ = auxInventoryEntity_->trait<Swan::InventoryTrait>();
		if (!auxInventory_) {
			auxInventoryEntity_ = {};
			closeInventoryCallback_ = nullptr;
			ui_.hoveredAuxInventorySlot = -1;
		}
	}

	// Draw auxiliary inventory
	if (auxInventory_) {
		auto content = auxInventory_->content();
		auto size = Swan::UI::calcInventorySize(content.size());
		ui_.auxInventoryRect = rnd.uiView({
			.pos = {0, 0},
			.size = size.add(2, 2),
		}, [&] {
			Swan::UI::inventory(
				ctx, rnd, size, inventorySprite_, content,
				ui_.hoveredAuxInventorySlot);
		}, Cygnet::Anchor::TOP);
	}

	// Draw tooltips for player inventory
	if (ui_.hoveredInventorySlot >= 0 && heldStack_.empty()) {
		auto &stack = inventory_.content_[ui_.hoveredInventorySlot];
		if (!stack.empty()) {
			Swan::UI::tooltip(ctx, rnd, stack.item()->displayName);
		}
	}

	// Draw tooltips for auxiliary inventory
	if (ui_.hoveredAuxInventorySlot >= 0 && heldStack_.empty()) {
		auto stack = auxInventory_->get(ui_.hoveredAuxInventorySlot);
		if (!stack.empty()) {
			Swan::UI::tooltip(ctx, rnd, stack.item()->displayName);
		}
	}

	// Everything after this is inventory stuff
	if (!ui_.showInventory) {
		return;
	}

	// Draw the rest of the inventory
	ui_.inventoryRect = rnd.uiView({
		.pos = {0, -2},
		.size = {12, 5},
	}, [&] {
		// Inventory content
		Swan::UI::inventory(
			ctx, rnd, {10, 3}, inventorySprite_,
			{inventory_.content_.begin() + 10, inventory_.content_.end()},
			ui_.hoveredInventorySlot - 10);

		// Selection
		if (ui_.selectedInventorySlot >= 10) {
			int slot = ui_.selectedInventorySlot - 10;
			int y = slot / 10;
			int x = slot % 10;
			rnd.drawUISprite({
				.transform = Cygnet::Mat3gf{}.translate(
					{float(x), float(y)}),
				.sprite = selectedSlotSprite_,
			}, Cygnet::Anchor::TOP_LEFT);
		}
	}, Cygnet::Anchor::BOTTOM);
}

void PlayerEntity::update(Swan::Ctx &ctx, float dt)
{
	if (interactTimer_ > 0) {
		interactTimer_ -= dt;
	}

	if (blackout_ > 0) {
		float prevBlackout = blackout_;
		blackout_ -= dt;
		if (prevBlackout > 1.5 && blackout_ <= 1.5) {
			ctx.game.playSound(sounds_.teleport);
			physicsBody_.body.pos = spawnPoint_;
			physicsBody_.vel = {};
			state_ = State::IDLE;
			currentAnimation_ = &sprites_.idle;
		}

		if (blackout_ > 1.5) {
			return;
		}
	}

	if (invulnerable_ > 0) {
		invulnerable_ -= dt;
	}

	if (health_ <= 0) {
		vit_ = Vit::LETHARGIC;
	}
	else if (health_ <= 4) {
		vit_ = Vit::WINDED;
	}
	else {
		vit_ = Vit::OK;
	}

	Swan::Vec2 facePos = physicsBody_.body.topMid() + Swan::Vec2{0, 0.3};
	Swan::Vec2 mousePos = ctx.game.getMousePos();
	auto lookVector = mousePos - facePos;
	auto raycast = ctx.plane.tiles().raycast(
		facePos, lookVector, std::min(lookVector.length(), 5.9f));
	breakPos_ = raycast.pos;
	placePos_ = raycast.pos + raycast.face;

	jumpTimer_.tick(dt);

	// Handle teleporting back to spawn point + animation
	if (teleState_ == 1) {
		teleportTimer_ -= dt;
		if (teleportTimer_ <= 0) {
			physicsBody_.body.pos = spawnPoint_;
			teleportTimer_ = 0;
			teleState_ = 2;
		}
	}
	else if (teleState_ == 2) {
		teleportTimer_ += dt * 2;
		if (teleportTimer_ >= 0.2) {
			teleState_ = 0;
		}
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_B)) {
		ctx.game.playSound(sounds_.teleport);
		teleportTimer_ = 0.2;
		teleState_ = 1;
	}

	handleInventorySelection(ctx);
	handleInventoryHover(ctx);

	if (ctx.game.wasKeyPressed(GLFW_KEY_M)) {
		health_ += 1;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_N)) {
		hurt(ctx, 1);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_P)) {
		auto &tile = ctx.plane.tiles().get(breakPos_);
		if (tile.onWorldTick) {
			Swan::info << "World ticking " << tile.name << " at " << breakPos_;
			tile.onWorldTick(ctx, breakPos_);
		}
	}

	// Break block, or click UI
	if (ctx.game.wasMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
		if (ui_.hoveredInventorySlot >= 0 || ui_.hoveredAuxInventorySlot >= 0) {
			handleInventoryClick(ctx);
		} else {
			onLeftClick(ctx);
		}
	}
	else if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
		if (ui_.hoveredInventorySlot < 0 && ui_.hoveredAuxInventorySlot < 0) {
			onLeftClick(ctx);
		}
	}

	// Place block, or activate tile or item
	if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
		onRightClick(ctx);
	}

	// Drop item
	if (ctx.game.wasKeyPressed(GLFW_KEY_Q)) {
		dropItem(ctx);
	}

	// God mode, or normal physics
	if (ctx.game.debug_.godMode) {
		physicsBody_.onGround = false;
		physicsBody_.friction();

		if (ctx.game.isKeyPressed(GLFW_KEY_W)) {
			physicsBody_.force += {0, -MOVE_FORCE_GROUND};
		}
		if (ctx.game.isKeyPressed(GLFW_KEY_A)) {
			physicsBody_.force += {-MOVE_FORCE_GROUND, 0};
		}
		if (ctx.game.isKeyPressed(GLFW_KEY_S)) {
			physicsBody_.force += {0, MOVE_FORCE_GROUND};
		}
		if (ctx.game.isKeyPressed(GLFW_KEY_D)) {
			physicsBody_.force += {MOVE_FORCE_GROUND, 0};
		}

		physicsBody_.updateNoclip(ctx, dt);
	} else  {
		handlePhysics(ctx, dt);

		bool wasOnGround = physicsBody_.onGround;
		auto oldVel = physicsBody_.vel;
		physicsBody_.update(ctx, dt);
		if (!wasOnGround && physicsBody_.onGround) {
			auto squareSpeed = oldVel.squareLength();
			if (squareSpeed >= 30 * 30) {
				hurt(ctx, 4);
			}
			else if (squareSpeed >= 25 * 25) {
				hurt(ctx, 3);
			}
			else if (squareSpeed >= 20 * 20) {
				hurt(ctx, 2);
			}
		}
	}
}

void PlayerEntity::tick(Swan::Ctx &ctx, float dt)
{
	auto lightPos = physicsBody_.body.topMid().as<int>();
	if (lightPos.x < 0) lightPos.x -= 1;
	if (lightPos.y < 0) lightPos.y -= 1;

	auto lightLevel = ctx.plane.tiles().getLightLevel(lightPos);
	float desiredGamma = 1.0 / ((lightLevel / 256.0) + 1) * 2;

	if (gamma_ < desiredGamma) {
		gamma_ += 0.01;
		if (gamma_ > desiredGamma) {
			gamma_ = desiredGamma;
		}
	} else if (gamma_ > desiredGamma) {
		gamma_ -= 0.01;
		if (gamma_ < desiredGamma) {
			gamma_ = desiredGamma;
		}
	}

	// Calculate the held light we would expect to produce
	std::optional<HeldLight> light;
	if (!heldStack_.empty() && heldStack_.item()->lightLevel) {
		light = {
			.pos = placePos_,
			.level = heldStack_.item()->lightLevel,
		};
	}

	// If the actual held light is different than what we expect,
	// tell the light system to remove and add lights as needed
	if (heldLight_ != light) {
		if (heldLight_) {
			ctx.plane.lights().removeLight(heldLight_->pos, heldLight_->level);
		}

		if (light) {
			ctx.plane.lights().addLight(light->pos, light->level);
		}

		heldLight_ = light;
	};

	if (auxInventory_ == &craftingInventory_) {
		craftingInventory_.recompute(ctx, inventory_.content());
	}
}

void PlayerEntity::serialize(
	Swan::Ctx &ctx, Proto::Builder w)
{
	physicsBody_.serialize(w.initBody());
	inventory_.serialize(w.initInventory());
	heldStack_.serialize(w.initHeldStack());
	auto sp = w.initSpawnPoint();
	sp.setX(spawnPoint_.x);
	sp.setY(spawnPoint_.y);
	w.setInventorySlot(ui_.selectedInventorySlot);
	w.setHealth(health_);
}

void PlayerEntity::deserialize(
	Swan::Ctx &ctx, Proto::Reader r)
{
	physicsBody_.deserialize(r.getBody());
	inventory_.deserialize(ctx, r.getInventory());
	heldStack_.deserialize(ctx, r.getHeldStack());

	if (r.hasSpawnPoint()) {
		auto sp = r.getSpawnPoint();
		spawnPoint_ = {sp.getX(), sp.getY()};
	}

	ui_.selectedInventorySlot = r.getInventorySlot();
	health_ = r.getHealth();
}

bool PlayerEntity::askToOpenInventory(
	Swan::EntityRef ent, CloseInventoryCallback cb)
{
	if (auxInventory_ && auxInventory_ != &craftingInventory_)
	{
		return false;
	}

	auxInventory_ = ent->trait<InventoryTrait>();
	if (!auxInventory_) {
		return false;
	}

	auxInventoryEntity_ = ent;
	closeInventoryCallback_ = cb;
	return true;
}

void PlayerEntity::askToCloseInventory(Swan::Ctx &ctx, Swan::EntityRef ent)
{
	if (auxInventoryEntity_ != ent) {
		Swan::info << "refusing to close inventory";
		return;
	}

	auxInventory_ = nullptr;
	auxInventoryEntity_ = {};
	auto cb = closeInventoryCallback_;
	closeInventoryCallback_ = nullptr;
	if (cb) {
		cb(ctx, ent);
	}
}

void PlayerEntity::hurt(Swan::Ctx &ctx, int n)
{
	if (invulnerable_ > 0) {
		return;
	}

	ctx.game.playSound(ctx.world.getSound("core::sounds/misc/hurt"), 0.2f * n);
	health_ -= n;
	if (health_ <= 0) {
		health_ = 0;
		blackout_ = BLACKOUT_TIME;
		state_ = State::IDLE;
		currentAnimation_ = &sprites_.idle;
	}

	invulnerable_ = 0.3;
}

bool PlayerEntity::heal(Swan::Ctx &, int n)
{
	if (health_ == MAX_HEALTH) {
		return false;
	}

	health_ += n;
	if (health_ > MAX_HEALTH) {
		health_ = MAX_HEALTH;
	}
	return true;
}

void PlayerEntity::onLeftClick(Swan::Ctx &ctx)
{
	if (interactTimer_ > 0) {
		return;
	}

	Swan::ToolSet tool = Swan::Tool::HAND;
	if (!heldStack_.empty() && heldStack_.item()->tool) {
		tool = heldStack_.item()->tool;
	}

	auto pos = breakPos_;
	bool canBreak = ctx.game.debug_.handBreakAny ||
		ctx.plane.tiles().get(pos).breakableBy.contains(tool);

	if (!canBreak) {
		pos = placePos_;
		canBreak = ctx.plane.tiles().get(pos).breakableBy.contains(tool);
	}

	if (!canBreak) {
		return;
	}

	breakTileAndDropItem(ctx, pos);
	interactTimer_ = 0.2;
}

void PlayerEntity::onRightClick(Swan::Ctx &ctx)
{
	if (interactTimer_ > 0) {
		return;
	}

	// First priority: activate the hovered tile if it can be activated
	Swan::Tile &hoveredTile = ctx.plane.tiles().get(breakPos_);
	if (hoveredTile.onActivate) {
		auto &stack = [&]() -> Swan::ItemStack & {
			if (!heldStack_.empty()) {
				return heldStack_;
			} else {
				return inventory_.content_[ui_.selectedInventorySlot];
			}
		}();
		Swan::Tile::ActivateMeta meta = {
			.activator = ctx.plane.entities().current(),
			.stack = stack,
		};
		hoveredTile.onActivate(ctx, breakPos_, meta);
		interactTimer_ = 0.2;
		return;
	}

	Swan::ItemStack &stack = heldStack_.empty()
		? inventory_.content_[ui_.selectedInventorySlot]
		: heldStack_;

	if (stack.empty()) {
		return;
	}

	Swan::Item &item = *stack.item();

	if (item.onActivate) {
		Swan::Vec2 origin = physicsBody_.body.topMid().add(0, 0.25);
		Swan::Vec2 mouse = ctx.game.getMousePos();
		Swan::Item::ActivateMeta meta = {
			.activator = ctx.plane.entities().current(),
			.stack = stack,
			.direction = (mouse - origin).norm(),
			.cursor = breakPos_,
		};
		item.onActivate(ctx, meta);
		interactTimer_ = 0.5;
		return;
	}

	if (!item.tile) {
		return;
	}

	if (
		item.tile->isSolid &&
		!ctx.plane.entities().getInTile(placePos_).empty()) {
		return;
	}

	if (!ctx.plane.placeTile(placePos_, item.tile->id)) {
		return;
	}

	// If we were holding a light emitting item,
	// and that stack is now empty,
	// remove the light from the held item
	if (heldStack_.count() == 1 && heldLight_) {
		ctx.plane.lights().removeLight(heldLight_->pos, heldLight_->level);
		heldLight_ = std::nullopt;
	}

	interactTimer_ = 0.2;
	stack.remove(1);
}

void PlayerEntity::dropItem(Swan::Ctx &ctx)
{
	Swan::ItemStack &stack = heldStack_.empty()
		? inventory_.content_[ui_.selectedInventorySlot]
		: heldStack_;

	if (stack.empty()) {
		return;
	}

	auto mousePos = ctx.game.getMousePos();
	auto pos = physicsBody_.body.topMid() + Swan::Vec2{0, 0.5};
	auto direction = (mousePos - pos).norm();
	auto vel = direction * 10.0;

	auto removed = stack.remove(1);
	ctx.plane.entities().spawn<ItemStackEntity>(pos, vel, removed.item());
}

void PlayerEntity::handleInventoryHover(Swan::Ctx &ctx)
{
	ui_.hoveredInventorySlot = -1;
	auto mousePos = ctx.game.getMouseUIPos();
	ui_.hoveredInventorySlot = Swan::UI::inventoryCellIndex(mousePos, ui_.hotbarRect);
	if (ui_.hoveredInventorySlot < 0 && ui_.showInventory) {
		ui_.hoveredInventorySlot = Swan::UI::inventoryCellIndex(mousePos, ui_.inventoryRect, 10);
	}
	ui_.hoveredAuxInventorySlot = -1;
	if (auxInventory_) {
		ui_.hoveredAuxInventorySlot = Swan::UI::inventoryCellIndex(
			mousePos, ui_.auxInventoryRect);
	}
}

void PlayerEntity::handlePhysics(Swan::Ctx &ctx, float dt)
{
	// Collide with stuff
	bool pickedUpItem = false;
	for (auto &c: ctx.plane.entities().getColliding(physicsBody_.body)) {
		auto *entity = c.ref.get();

		// Pick it up if it's an item stack, and don't collide
		auto *itemStackEnt = dynamic_cast<ItemStackEntity *>(entity);
		if (itemStackEnt) {
			// Don't pick up immediately
			if (itemStackEnt->lifetime_ < 0.2) {
				continue;
			}

			// Only one per update
			if (pickedUpItem) {
				continue;
			}

			Swan::ItemStack stack{itemStackEnt->item(), 1};
			stack = inventory_.insert(stack);
			if (stack.empty()) {
				ctx.plane.entities().despawn(c.ref);
				ctx.game.playSound(sounds_.snap);
				pickedUpItem = true;
			}
			continue;
		}

		if (c.body.isSolid) {
			physicsBody_.collideWith(c.body);
		}

		// Get damaged if it's something which deals contact damage
		auto *damage = entity->trait<Swan::ContactDamageTrait>();
		bool damaged = false;
		if (damage && invincibleTimer_ <= 0) {
			Swan::Vec2 direction;
			direction.y = -0.5;
			if (physicsBody_.body.center().x < c.body.center().x) {
				direction.x = -1;
			}
			else {
				direction.x = 1;
			}

			physicsBody_.vel += direction * damage->knockback;
			damaged = true;
		}

		if (damaged) {
			invincibleTimer_ = 0.5;
		}
	}

	// Figure out what tile we're in
	auto midTilePos = Swan::TilePos{
		(int)floor(physicsBody_.body.midX()),
		(int)floor(physicsBody_.body.bottom() - 0.1),
	};
	auto &midTile = ctx.plane.tiles().get(midTilePos);
	auto &topTile = ctx.plane.tiles().get(midTilePos.add(0, -1));

	// Figure out what tile is below us
	auto belowTilePos = Swan::TilePos{
		(int)floor(physicsBody_.body.midX()),
		(int)floor(physicsBody_.body.bottom() + 0.1),
	};
	auto &belowTile = ctx.plane.tiles().get(belowTilePos);

	// Find a speed multiplier based on vitality
	float speedMult = 1;
	switch (vit_) {
	case Vit::OK:
		sprites_.idle.setInterval(0.2);
		break;
	case Vit::WINDED:
		sprites_.idle.setInterval(0.25);
	case Vit::LETHARGIC:
		sprites_.idle.setInterval(0.4);
		speedMult = 0.7;
		break;
	}

	bool inLadder =
		dynamic_cast<LadderTileTrait *>(midTile.traits.get()) ||
		dynamic_cast<LadderTileTrait *>(topTile.traits.get());

	auto fluidCenterPos = Swan::Vec2{
		physicsBody_.body.midX(),
		physicsBody_.body.top() + 0.25f,
	};

	auto fluidBottomPos = Swan::Vec2{
		physicsBody_.body.midX(),
		physicsBody_.body.bottom() - 0.25f,
	};

	// Figure out what fluids we're in
	Swan::Fluid &fluidTop = ctx.plane.fluids().getAtPos(
		physicsBody_.body.topMid().add(0, 0.1));
	Swan::Fluid &fluidCenter = ctx.plane.fluids().getAtPos(fluidCenterPos);
	Swan::Fluid &fluidBottom = ctx.plane.fluids().getAtPos(fluidBottomPos);

	// Drown
	if (fluidTop.id == Swan::World::AIR_FLUID_ID) {
		oxygen_ += dt * 4;
		if (oxygen_ > MAX_OXYGEN) {
			oxygen_ = MAX_OXYGEN;
		}
	}
	else if (blackout_ <= 0) {
		oxygen_ -= dt * 0.75;
		if (oxygen_ < 0) {
			ctx.game.playSound(ctx.world.getSound("core::sounds/misc/hurt"), 0.4f);
			oxygen_ = 0;
			health_ = 0;
			blackout_ = BLACKOUT_TIME;
			state_ = State::IDLE;
			currentAnimation_ = &sprites_.idle;
		}
	}

	{ // Splash sound
		bool oldInFluid = inFluid_;
		if (fluidCenter.density > 0 && fluidBottom.density > 0) {
			inFluid_ = true;
			fluidColor_ = fluidCenter.fg;
		}
		else if (fluidCenter.density <= 0 && fluidBottom.density <= 0) {
			inFluid_ = false;
		}

		if (inFluid_ && !oldInFluid) {
			ctx.game.playSound(sounds_.splash);
			for (int i = 0; i < 60; ++i) {
				ctx.game.spawnParticle({
					.pos = fluidCenterPos + Swan::Vec2{
						(Swan::randfloat() - 0.5f) * 0.2f,
						(Swan::randfloat() - 0.5f) * 0.2f + 0.2f,
					},
					.vel = {
						(Swan::randfloat() * 6 - 3) + (physicsBody_.vel.x * 0.5f),
						Swan::randfloat() * -3 - 4,
					},
					.color = fluidColor_,
					.lifetime = 0.4f + Swan::randfloat() * 0.2f,
				});
			}
		}
		else if (!inFluid_ && oldInFluid) {
			ctx.game.playSound(sounds_.shortSplash);
			for (int i = 0; i < 40; ++i) {
				ctx.game.spawnParticle({
					.pos = fluidBottomPos + Swan::Vec2{
						(Swan::randfloat() - 0.5f) * 0.3f,
						(Swan::randfloat() - 0.6f) * 0.3f + 0.4f,
					},
					.vel = {
						(Swan::randfloat() * 4 - 2) + (physicsBody_.vel.x * 0.5f),
						Swan::randfloat() * -2 - 6,
					},
					.color = fluidColor_,
					.lifetime = 0.4f + Swan::randfloat() * 0.2f,
				});
			}
		}
	}

	// Handle sprint press
	if (ctx.game.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
		sprinting_ = true;
	}
	if (vit_ == Vit::LETHARGIC) {
		sprinting_ = false;
	}

	// Handle left/right key press
	int runDirection = 0;
	if (ctx.game.isKeyPressed(GLFW_KEY_A)) {
		runDirection -= 1;
	}
	if (ctx.game.isKeyPressed(GLFW_KEY_D)) {
		runDirection += 1;
	}

	// Handle ladder climb
	if (inLadder) {
		if (ctx.game.isKeyPressed(GLFW_KEY_W)) {
			physicsBody_.force += Swan::Vec2{0, -LADDER_CLIMB_FORCE};
		}
		else if (ctx.game.isKeyPressed(GLFW_KEY_S)) {
			physicsBody_.force += Swan::Vec2{0, LADDER_CLIMB_FORCE};
		}

		if (physicsBody_.vel.y > LADDER_MAX_VEL) {
			physicsBody_.vel.y = LADDER_MAX_VEL;
		}
		else if (physicsBody_.vel.y < -LADDER_MAX_VEL && physicsBody_.force.y < 0) {
			physicsBody_.force.y = 0;
		}
	}

	float moveForce;
	if (physicsBody_.onGround && sprinting_) {
		moveForce = SPRINT_FORCE_GROUND;
	}
	else if (physicsBody_.onGround) {
		moveForce = MOVE_FORCE_GROUND;
	}
	else {
		moveForce = MOVE_FORCE_AIR;
	}
	moveForce *= speedMult;

	State oldState = state_;
	state_ = State::IDLE;

	// Act on run direction
	if (runDirection < 0) {
		state_ = State::RUNNING;
		lastDirection_ = -1;
		physicsBody_.force += Swan::Vec2(-moveForce, 0);
	}
	else if (runDirection > 0) {
		state_ = State::RUNNING;
		lastDirection_ = 1;
		physicsBody_.force += Swan::Vec2(moveForce, 0);
	}
	else {
		state_ = State::IDLE;
		sprinting_ = false;
	}

	// Adjust running speed based on sprinting
	sprites_.running.setInterval((sprinting_ ? 0.07 : 0.1) / speedMult);

	// If we hit the ground, override the desired state to be landing
	if (physicsBody_.onGround && (oldState == State::FALLING || oldState == State::JUMPING)) {
		state_ = State::LANDING;
	}

	// Don't switch away from landing unless it's done!
	if (oldState == State::LANDING && !sprites_.landing.done()) {
		state_ = State::LANDING;
	}

	bool jumpPressed = ctx.game.isKeyPressed(GLFW_KEY_SPACE);

	// Jump
	if (physicsBody_.onGround && jumpPressed && jumpTimer_.periodic(0.5)) {
		physicsBody_.vel.y = -JUMP_VEL;
	}

	// Fall down faster than we went up
	if (
		!inLadder &&
		!physicsBody_.onGround &&
		(!jumpPressed || physicsBody_.vel.y > 0)) {
		physicsBody_.force += Swan::Vec2(0, DOWN_FORCE);
	}

	// Show falling or jumping animation depending on whether we're going up or down
	if (!physicsBody_.onGround) {
		if (oldState == State::JUMPING || oldState == State::FALLING) {
			state_ = oldState;
		}
		else if (physicsBody_.vel.y < -0.1) {
			state_ = State::JUMPING;
		}
		else {
			state_ = State::FALLING;
		}
	}

	if (state_ != oldState) {
		switch (state_) {
		case State::IDLE:
			currentAnimation_ = &sprites_.idle;
			break;

		case State::RUNNING:
			currentAnimation_ = &sprites_.running;
			break;

		case State::FALLING:
			currentAnimation_ = &sprites_.falling;
			break;

		case State::JUMPING:
			currentAnimation_ = &sprites_.jumping;
			break;

		case State::LANDING:
			currentAnimation_ = &sprites_.landing;
			break;
		}
		currentAnimation_->reset();
	}
	currentAnimation_->tick(dt);

	if (invincibleTimer_ >= 0) {
		invincibleTimer_ -= dt;
	}

	if (state_ == State::RUNNING) {
		stepTimer_ -= dt;
		if (stepTimer_ <= 0) {
			auto *sound = belowTile.stepSounds[stepIndex_];
			ctx.game.playSound(sound, 0.2);
			stepIndex_ = (stepIndex_ + 1) % 2;
			stepTimer_ += (sprinting_ ? 0.28 : 0.4) / speedMult;
		}
	}
	else if (state_ == State::LANDING && oldState != State::LANDING) {
		auto *sound = belowTile.stepSounds[stepIndex_];
		stepIndex_ = (stepIndex_ + 1) % 2;
		ctx.game.playSound(sound, 0.2);
		stepTimer_ = 0.2;
	}
	else {
		stepTimer_ = 0.15;
	}

	if (inLadder && ctx.game.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
		physicsBody_.friction({1000, 1000});
	}
	else {
		float centerDensity = std::min(fluidCenter.density, 10.0f);
		float bottomDensity = std::min(fluidBottom.density, 10.0f);

		physicsBody_.standardForces();
		physicsBody_.friction(Swan::Vec2{200, 400} * centerDensity);

		float gForce = Swan::BasicPhysicsBody::GRAVITY * physicsBody_.mass + DOWN_FORCE;

		if (ctx.game.isKeyPressed(GLFW_KEY_SPACE)) {
			physicsBody_.force.y -= gForce * bottomDensity * 0.8;
			physicsBody_.force -= Swan::Vec2{0, SWIM_FORCE_UP * bottomDensity};
		}
		else if (
			ctx.game.isKeyPressed(GLFW_KEY_S) ||
			ctx.game.isKeyPressed(GLFW_KEY_DOWN)) {
			physicsBody_.force.y -= gForce * centerDensity * 0.8;
			physicsBody_.force += Swan::Vec2{0, SWIM_FORCE_DOWN * centerDensity};
		}
		else {
			physicsBody_.force.y -= gForce * centerDensity * 0.8;
		}
	}
}

void PlayerEntity::handleInventoryClick(Swan::Ctx &ctx)
{
	auto clickInventory = [&](Swan::InventoryTrait::Inventory &inv, int index) {
		auto slot = inv.get(index);
		if (slot.empty() && heldStack_.empty()) {
			return;
		}

		if (&inv == &craftingInventory_ && heldStack_.item() == slot.item()) {
			if (heldStack_.count() + slot.count() <= slot.item()->maxStack) {
				heldStack_.insert(inv.take(index));
			}
		}
		else if (heldStack_.empty() && !slot.empty()) {
			heldStack_ = inv.take(index);
			slot = {};
		}
		else if (!heldStack_.empty() && slot.empty()) {
			heldStack_ = inv.set(index, heldStack_);
		}
		else if (slot.item() == heldStack_.item()) {
			heldStack_ = inv.insert(heldStack_, index, index + 1);
		}
		else {
			heldStack_ = inv.set(index, heldStack_);
			if (heldStack_.empty()) {
				heldStack_ = slot;
			}
		}
	};

	if (ui_.hoveredInventorySlot < 0 && ui_.hoveredAuxInventorySlot < 0) {
		return;
	}

	bool shift = ctx.game.isKeyPressed(GLFW_KEY_LEFT_SHIFT);

	// Non-shift click on the player inventory
	if (!shift && ui_.hoveredInventorySlot >= 0) {
		clickInventory(inventory_, ui_.hoveredInventorySlot);
		ctx.game.playSound(sounds_.snap);
		return;
	}

	// Shift-click from player inventory to aux inventory
	if (
			shift && auxInventory_ && ui_.hoveredInventorySlot >= 0 &&
			auxInventory_ != &craftingInventory_) {
		int index = ui_.hoveredInventorySlot;
		inventory_.set(index, auxInventory_->insert(inventory_.get(index)));
		ctx.game.playSound(sounds_.snap);
		return;
	}

	// Shift click from aux inventory to player inventory
	if (shift && auxInventory_ && ui_.hoveredAuxInventorySlot >= 0) {
		int index = ui_.hoveredAuxInventorySlot;
		if (ui_.showInventory) {
			auxInventory_->set(index, inventory_.insert(auxInventory_->take(index), 10));
		}
		auxInventory_->set(index, inventory_.insert(auxInventory_->take(index)));
		ctx.game.playSound(sounds_.snap);
		return;
	}

	// Shift click from player inventory to player inventory
	if (shift && ui_.hoveredInventorySlot >= 0) {
		int index = ui_.hoveredInventorySlot;
		auto stack = inventory_.take(index);
		Swan::ItemStack leftover;
		if (index < 10) {
			leftover = inventory_.insert(stack, 10);
		} else {
			leftover = inventory_.insert(stack, 0, 10);
		}
		inventory_.insert(leftover);
		ctx.game.playSound(sounds_.snap);
		return;
	}

	// Non-shift click on the open inventory
	if (!shift && auxInventory_ && ui_.hoveredAuxInventorySlot >= 0) {
		clickInventory(*auxInventory_, ui_.hoveredAuxInventorySlot);
		ctx.game.playSound(sounds_.snap);
		return;
	}
}

void PlayerEntity::handleInventorySelection(Swan::Ctx &ctx)
{
	// Select item slots directly
	auto selectSlot = [&](int slot) {
		int delta = slot - (ui_.selectedInventorySlot % 10);
		ui_.selectedInventorySlot += delta;
	};
	if (ctx.game.wasKeyPressed(GLFW_KEY_1)) {
		selectSlot(0);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_2)) {
		selectSlot(1);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_3)) {
		selectSlot(2);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_4)) {
		selectSlot(3);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_5)) {
		selectSlot(4);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_6)) {
		selectSlot(5);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_7)) {
		selectSlot(6);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_8)) {
		selectSlot(7);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_9)) {
		selectSlot(8);
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_0)) {
		selectSlot(9);
	}

	// Navigate left/right/up/down
	if (ctx.game.wasKeyPressed(GLFW_KEY_LEFT)) {
		if (ui_.selectedInventorySlot % 10 == 0) {
			ui_.selectedInventorySlot += 9;
		} else {
			ui_.selectedInventorySlot -= 1;
		}
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_RIGHT)) {
		if (ui_.selectedInventorySlot % 10 == 9) {
			ui_.selectedInventorySlot -= 9;
		} else {
			ui_.selectedInventorySlot += 1;
		}
	}
	else if (
		ui_.showInventory && (
			ctx.game.wasKeyPressed(GLFW_KEY_UP) ||
			ctx.game.wasKeyPressed(GLFW_KEY_V))) {
		if (ui_.selectedInventorySlot < 10) {
			ui_.selectedInventorySlot += INVENTORY_SIZE - 10;
		} else {
			ui_.selectedInventorySlot -= 10;
		}
		if (ui_.selectedInventorySlot < 0) {
			ui_.selectedInventorySlot += 10;
		}
	}
	else if (
		ui_.showInventory && (
			ctx.game.wasKeyPressed(GLFW_KEY_DOWN) ||
			ctx.game.wasKeyPressed(GLFW_KEY_C))) {
		if (ui_.selectedInventorySlot >= INVENTORY_SIZE - 10) {
			ui_.selectedInventorySlot -= INVENTORY_SIZE - 10;
		} else {
			ui_.selectedInventorySlot += 10;
		}
		if (ui_.selectedInventorySlot >= INVENTORY_SIZE) {
			ui_.selectedInventorySlot -= INVENTORY_SIZE;
		}
	}

	// Handle held items
	if (ctx.game.wasKeyPressed(GLFW_KEY_X)) {
		Swan::ItemStack &slot = inventory_.content_[ui_.selectedInventorySlot];
		if (heldStack_.empty() && !slot.empty()) {
			ctx.game.playSound(sounds_.snap);
			heldStack_ = slot;
			slot = {};
		}
		else if (!heldStack_.empty()) {
			ctx.game.playSound(sounds_.snap);
			auto tmp = heldStack_;
			heldStack_ = slot.insert(heldStack_);
			if (heldStack_ == tmp) {
				heldStack_ = slot;
				slot = tmp;
			}
		}
	}

	// Toggle inventory
	if (ctx.game.wasKeyPressed(GLFW_KEY_E)) {
		if (ui_.showInventory) {
			ctx.game.playSound(sounds_.inventoryClose, 0.2);
			ui_.showInventory = false;
			ui_.selectedInventorySlot %= 10;
			if (auxInventory_ && auxInventory_ != &craftingInventory_) {
				if (closeInventoryCallback_ && auxInventoryEntity_) {
					closeInventoryCallback_(ctx, auxInventoryEntity_);
				}
				auxInventoryEntity_ = {};
				closeInventoryCallback_ = nullptr;
				auxInventory_ = nullptr;
			}
		}
		else {
			ctx.game.playSound(sounds_.inventoryOpen);
			ui_.showInventory = true;
		}
	}

	// Toggle crafting menu
	if (ctx.game.wasKeyPressed(GLFW_KEY_F)) {
		if (auxInventory_ == &craftingInventory_) {
			auxInventory_ = nullptr;
		}
		else {
			if (closeInventoryCallback_ && auxInventoryEntity_) {
				closeInventoryCallback_(ctx, auxInventoryEntity_);
			}
			auxInventoryEntity_ = {};
			closeInventoryCallback_ = nullptr;
			auxInventory_ = &craftingInventory_;
			craftingInventory_.recompute(ctx, inventory_.content());
		}
	}
}

}
