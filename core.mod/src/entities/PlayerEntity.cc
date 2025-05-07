#include "PlayerEntity.h"

#include <algorithm>
#include <stdio.h>
#include <imgui/imgui.h>
#include <string>
#include <unordered_map>

#include "ItemStackEntity.h"
#include "world/util.h"
#include "world/ladder.h"

namespace CoreMod {

PlayerEntity::PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos):
	PlayerEntity(ctx)
{
	physicsBody_.body.pos = pos;
	spawnPoint_ = pos.as<int>();
}

void PlayerEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd)
{
	rnd.setGamma(gamma_);

	Cygnet::Mat3gf mat;

	// Currently, there is no sprite for running left.
	// Running left is just running right but flipped.
	if (lastDirection_ < 0) {
		mat.translate({-0.9, 0}).scale({-1, 1}).translate({0.9, 0});
	}

	// Teleportation animation..
	if (teleportTimer_ > 0) {
		float frac = teleportTimer_ / 0.2;
		mat.scale({frac, 1});
		mat.translate({(1.0f - frac) * 0.9f, 0});
	}

	// The running animation dips into the ground a bit
	if (state_ == State::RUNNING) {
		mat.translate({0, 4.0 / 32.0});
	}

	// Position
	mat.translate(physicsBody_.body.pos - Swan::Vec2{0.6, 0.5});

	currentAnimation_->draw(rnd, mat);

	rnd.drawRect({Swan::Vec2(placePos_).add(0.1, 0.1), {0.8, 0.8}});
	rnd.drawRect({breakPos_, {1, 1}});

	// Draw hotbar
	ui_.hotbarRect = rnd.uiView({
		.size = {12, 3},
	}, [&] {
		// Hotbar content
		Swan::UI::inventory(ctx, rnd, {10, 1}, inventorySprite_, inventory_.content);

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
			.text = std::to_string(heldStack_.count()).c_str(),
		});
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
			{inventory_.content.begin() + 10, inventory_.content.end()});

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

	// This whole method is stupid inefficient and does a bunch of memory allocation every frame.
	// TODO: fix.

	std::unordered_map<Swan::Item *, int> itemCounts;
	for (size_t i = 0; i < inventory_.content.size(); ++i) {
		auto &stack = inventory_.content[i];
		if (stack.empty()) {
			continue;
		}

		itemCounts[stack.item()] += stack.count();
	}

	ImGui::Begin("Crafting");
	std::string text;
	for (auto &recipe: ctx.world.getRecipes("core::crafting")) {
		bool craftable = true;
		for (const auto &input: recipe.inputs) {
			if (!itemCounts.contains(input.item)) {
				craftable = false;
				break;
			}

			if (itemCounts[input.item] < input.count) {
				craftable = false;
				break;
			}
		}

		if (!craftable) {
			continue;
		}

		text.clear();
		text += "Craft ";
		text += std::to_string(recipe.output.count);
		text += ' ';
		text += recipe.output.item->name;
		text += " from ";

		bool first = true;
		for (const auto &input: recipe.inputs) {
			if (!first) {
				text += ", ";
			}
			first = false;

			text += std::to_string(input.count);
			text += ' ';
			text += input.item->name;
		}

		if (ImGui::Button(text.c_str())) {
			craft(ctx, recipe);
		}
	}
	ImGui::End();
}

void PlayerEntity::update(const Swan::Context &ctx, float dt)
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

	if (interactTimer_ >= 0) {
		interactTimer_ -= dt;
	}

	State oldState = state_;

	state_ = State::IDLE;

	Swan::Vec2 facePos = physicsBody_.body.topMid() + Swan::Vec2{0, 0.3};
	Swan::Vec2 mousePos = ctx.game.getMousePos();
	auto lookVector = mousePos - facePos;
	auto raycast = ctx.plane.tiles().raycast(
		facePos, lookVector, std::min(lookVector.length(), 5.9f));
	breakPos_ = raycast.pos;
	placePos_ = raycast.pos + raycast.face;

	jumpTimer_.tick(dt);

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
	Swan::Fluid &fluidCenter = ctx.plane.fluids().getAtPos(fluidCenterPos);
	Swan::Fluid &fluidBottom = ctx.plane.fluids().getAtPos(fluidBottomPos);

	{ // Splash sound
		bool oldInFluid = inFluid_;
		if (fluidCenter.density > 0 && fluidBottom.density > 0) {
			inFluid_ = true;
			fluidColor_ = fluidCenter.color;
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

	// Go back to spawn point
	if (teleportTimer_ > 0) {
		teleportTimer_ -= dt;
		if (teleportTimer_ <= 0) {
			physicsBody_.body.pos = spawnPoint_;
		}
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_B)) {
		ctx.game.playSound(sounds_.teleport);
		teleportTimer_ = 0.2;
	}

	handleInventorySelection(ctx);
	handleInventoryHover(ctx);

	// Break block, or click UI
	if (ctx.game.wasMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
		if (!handleInventoryClick(ctx)) {
			onLeftClick(ctx);
		}
	}
	else if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
		if (ui_.hoveredInventorySlot < 0) {
			onLeftClick(ctx);
		}
	}

	// Place block, or activate item
	if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
		onRightClick(ctx);
	}

	// Drop item
	if (ctx.game.wasKeyPressed(GLFW_KEY_Q)) {
		dropItem(ctx);
	}

	// Handle sprint press
	if (ctx.game.isKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
		sprinting_ = true;
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
	animations_.running.setInterval(sprinting_ ? 0.07 : 0.1);

	// If we hit the ground, override the desired state to be landing
	if (physicsBody_.onGround && (oldState == State::FALLING || oldState == State::JUMPING)) {
		state_ = State::LANDING;
	}

	// Don't switch away from landing unless it's done!
	if (oldState == State::LANDING && !animations_.landing.done()) {
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
			currentAnimation_ = &animations_.idle;
			break;

		case State::RUNNING:
			currentAnimation_ = &animations_.running;
			break;

		case State::FALLING:
			currentAnimation_ = &animations_.falling;
			break;

		case State::JUMPING:
			currentAnimation_ = &animations_.jumping;
			break;

		case State::LANDING:
			currentAnimation_ = &animations_.landing;
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
			ctx.game.playSound(sound);

			stepIndex_ = (stepIndex_ + 1) % 2;
			stepTimer_ += sprinting_ ? 0.28 : 0.4;
		}
	}
	else if (state_ == State::LANDING && oldState != State::LANDING) {
		auto *sound = belowTile.stepSounds[1];
		stepIndex_ = 0;
		ctx.game.playSound(sound);
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

	physicsBody_.update(ctx, dt);
}

void PlayerEntity::tick(const Swan::Context &ctx, float dt)
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
}

void PlayerEntity::serialize(
	const Swan::Context &ctx, Proto::Builder w)
{
	physicsBody_.serialize(w.initBody());
	inventory_.serialize(w.initInventory());
	heldStack_.serialize(w.initHeldStack());
	auto sp = w.initSpawnPoint();
	sp.setX(spawnPoint_.x);
	sp.setY(spawnPoint_.y);
}

void PlayerEntity::deserialize(
	const Swan::Context &ctx, Proto::Reader r)
{
	physicsBody_.deserialize(r.getBody());
	inventory_.deserialize(ctx, r.getInventory());
	heldStack_.deserialize(ctx, r.getHeldStack());

	if (r.hasSpawnPoint()) {
		auto sp = r.getSpawnPoint();
		spawnPoint_ = {sp.getX(), sp.getY()};
	}
}

void PlayerEntity::onLeftClick(const Swan::Context &ctx)
{
	if (interactTimer_ > 0) {
		return;
	}

	Swan::ToolSet tool = Swan::Tool::HAND;
	if (!heldStack_.empty() && heldStack_.item()->tool) {
		tool = heldStack_.item()->tool;
	}

	auto pos = breakPos_;
	bool canBreak = ctx.game.debugHandBreakAny_ ||
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

void PlayerEntity::onRightClick(const Swan::Context &ctx)
{
	if (interactTimer_ > 0) {
		return;
	}

	Swan::ItemStack &stack = heldStack_.empty()
		? inventory_.content[ui_.selectedInventorySlot]
		: heldStack_;

	if (stack.empty()) {
		return;
	}

	Swan::Item &item = *stack.item();

	if (item.onActivate) {
		Swan::Vec2 origin = physicsBody_.body.topMid().add(0, 0.25);
		Swan::Vec2 mouse = ctx.game.getMousePos();
		item.onActivate(ctx, stack, origin, (mouse - origin).norm());
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

void PlayerEntity::craft(const Swan::Context &ctx, const Swan::Recipe &recipe)
{
	// Back up the state of the inventory,
	// so that we can undo everything if it turns out we don't have enough items
	Swan::BasicInventory backup = inventory_;

	for (const auto &input: recipe.inputs) {
		int needed = input.count;
		for (auto &slot: inventory_.content) {
			if (slot.empty() || slot.item() != input.item) {
				continue;
			}

			auto got = slot.remove(needed);
			needed -= got.count();
			if (needed == 0) {
				break;
			}
		}

		if (needed != 0) {
			Swan::info
				<< "Attempted to craft " << recipe.output.item->name
				<< ", but missing " << needed << ' ' << input.item;
			inventory_ = backup;
			return;
		}
	}

	Swan::ItemStack stack(recipe.output.item, recipe.output.count);
	stack = inventory_.insert(stack);
	if (stack.empty()) {
		ctx.game.playSound(sounds_.crafting, 0.2);
	} else {
		Swan::info
			<< "Attempted to craft " << recipe.output.item->name
			<< ", but there wasn't enough space in the inventory to put the result";
		inventory_ = backup;
	}
}

void PlayerEntity::dropItem(const Swan::Context &ctx)
{
	Swan::ItemStack &stack = heldStack_.empty()
		? inventory_.content[ui_.selectedInventorySlot]
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

void PlayerEntity::handleInventoryHover(const Swan::Context &ctx)
{
	ui_.hoveredInventorySlot = -1;

	auto mousePos = ctx.game.getMouseUIPos();
	auto hotbarPos = Swan::UI::inventoryCellPos(mousePos, ui_.hotbarRect);
	if (hotbarPos) {
		ui_.hoveredInventorySlot = hotbarPos->x;
	} else if (ui_.showInventory) {
		auto invPos = Swan::UI::inventoryCellPos(mousePos, ui_.inventoryRect);
		if (invPos) {
			ui_.hoveredInventorySlot = 10 + invPos->y * 10 + invPos->x;
		}
	}
}

bool PlayerEntity::handleInventoryClick(const Swan::Context &ctx)
{
	if (ui_.hoveredInventorySlot < 0) {
		return false;
	}

	// I don't understand what this does anymore,
	// but it's the same as what happens when you press 'x'
	// and I think I want to keep that behavior..
	Swan::ItemStack &slot = inventory_.content[ui_.hoveredInventorySlot];
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

	return true;
}

void PlayerEntity::handleInventorySelection(const Swan::Context &ctx)
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
		Swan::ItemStack &slot = inventory_.content[ui_.selectedInventorySlot];
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
		} else {
			ctx.game.playSound(sounds_.inventoryOpen);
			ui_.showInventory = true;
		}
	}
}

}
