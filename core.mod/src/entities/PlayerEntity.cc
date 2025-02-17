#include "PlayerEntity.h"

#include <algorithm>
#include <stdio.h>
#include <imgui/imgui.h>
#include <unordered_map>

#include "ItemStackEntity.h"
#include "world/util.h"
#include "world/ladder.h"

namespace CoreMod {

PlayerEntity::PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos):
	PlayerEntity(ctx)
{
	physicsBody_.body.pos = pos;
}

void PlayerEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd)
{
	Cygnet::Mat3gf mat;

	// Currently, there is no sprite for running left.
	// Running left is just running right but flipped.
	if (lastDirection_ < 0) {
		mat.translate({-0.9, 0}).scale({-1, 1}).translate({0.9, 0});
	}

	// The running animation dips into the ground a bit
	if (state_ == State::RUNNING) {
		mat.translate({0, 4.0 / 32.0});
	}

	currentAnimation_->draw(rnd, mat.translate(
		physicsBody_.body.pos - Swan::Vec2{0.6, 0.5}));

	rnd.drawRect({Swan::Vec2(placePos_).add(0.1, 0.1), {0.8, 0.8}});
	rnd.drawRect({breakPos_, {1, 1}});

	rnd.drawUISprite({
		.transform = Cygnet::Mat3gf{}.translate({0, -1}),
		.sprite = hotbarSprite_,
	}, Cygnet::Anchor::BOTTOM);

	rnd.drawUISprite({
		.transform = Cygnet::Mat3gf{}.translate({
			-4.5f + selectedInventorySlot_, -1 + 2 / 32.0}),
		.sprite = selectedSlotSprite_,
	}, Cygnet::Anchor::BOTTOM);

	char stackSizeBuf[8];

	for (int i = 0; i < 10; ++i) {
		auto &stack = inventory_.content[i];
		if (stack.empty()) {
			continue;
		}

		rnd.drawUITile({
			.transform = Cygnet::Mat3gf{}
				.scale({0.6, 0.6})
				.translate({0.2, 0.2})
				.translate({-4.5f + i, -1}),
			.id = stack.item()->id,
		}, Cygnet::Anchor::BOTTOM);

		snprintf(stackSizeBuf, sizeof(stackSizeBuf), "%d", stack.count());

		auto &text = rnd.drawUIText({
			.textCache = ctx.game.smallFont_,
			.transform = Cygnet::Mat3gf{}
				.scale({0.7, 0.7})
				.translate({-4.875f + i, -0.95}),
			.text = stackSizeBuf,
		}, Cygnet::Anchor::BOTTOM);
		text.drawText.transform.translate({text.size.x / 2, 0});
	}

	if (!heldStack_.empty()) {
		rnd.drawUITile({
			.transform = Cygnet::Mat3gf{}
				.scale({1.5, 1.5})
				.translate({ctx.game.getMouseUIPos()})
				.translate({-0.75, -1.0}),
			.id = heldStack_.item()->id,
		}, Cygnet::Anchor::TOP_LEFT);

		snprintf(stackSizeBuf, sizeof(stackSizeBuf), "%d", heldStack_.count());

		rnd.drawUIText({
			.textCache = ctx.game.smallFont_,
			.transform = Cygnet::Mat3gf{}
				.scale({0.7, 0.7})
				.translate({ctx.game.getMouseUIPos()})
				.translate({0, 0.5}),
			.text = stackSizeBuf,
		}, Cygnet::Anchor::TOP_LEFT);
	}

	// Everything after this is inventory stuff
	if (!showInventory_) {
		return;
	}

	// This whole method is stupid inefficient and does a bunch of memory allocation every frame.
	// TODO: fix.

	ImGui::Begin("Inventory");
	auto &selectedStack = inventory_.content[selectedInventorySlot_];
	if (selectedStack.empty()) {
		ImGui::Text("Selected: [%d]: Empty", selectedInventorySlot_ + 1);
	}
	else {
		ImGui::Text("Selected: [%d]: %d x %s", selectedInventorySlot_ + 1,
			selectedStack.count(), selectedStack.item()->name.c_str());
	}

	std::unordered_map<Swan::Item *, int> itemCounts;
	for (size_t i = 0; i < inventory_.content.size(); ++i) {
		auto &stack = inventory_.content[i];
		if (stack.empty()) {
			continue;
		}

		itemCounts[stack.item()] += stack.count();
		char sel = ' ';
		if (i == (size_t)selectedInventorySlot_) {
			sel = 'x';
		}

		ImGui::Text("%c %zu: %d x %s", sel, i + 1, stack.count(), stack.item()->name.c_str());
	}
	ImGui::End();

	ImGui::Begin("Crafting");
	std::string text;
	for (const auto &recipe: ctx.world.recipes_) {
		if (recipe.kind != "crafting") {
			continue;
		}

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
			craft(recipe);
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
				ctx.game.playSound(snapSound_);
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
		facePos, lookVector, std::min(lookVector.length(), 6.0f));
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
			ctx.game.playSound(splashSound_);
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
			ctx.game.playSound(shortSplashSound_);
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

	// Select item slots
	if (ctx.game.wasKeyPressed(GLFW_KEY_1)) {
		selectedInventorySlot_ = 0;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_2)) {
		selectedInventorySlot_ = 1;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_3)) {
		selectedInventorySlot_ = 2;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_4)) {
		selectedInventorySlot_ = 3;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_5)) {
		selectedInventorySlot_ = 4;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_6)) {
		selectedInventorySlot_ = 5;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_7)) {
		selectedInventorySlot_ = 6;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_8)) {
		selectedInventorySlot_ = 7;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_9)) {
		selectedInventorySlot_ = 8;
	}
	else if (ctx.game.wasKeyPressed(GLFW_KEY_0)) {
		selectedInventorySlot_ = 9;
	}

	if (ctx.game.wasKeyPressed(GLFW_KEY_X)) {
		Swan::ItemStack &slot = inventory_.content[selectedInventorySlot_];
		if (heldStack_.empty()) {
			heldStack_ = slot;
			slot = {};
		}
		else {
			auto tmp = heldStack_;
			heldStack_ = slot.insert(heldStack_);
			if (heldStack_ == tmp) {
				heldStack_ = slot;
				slot = tmp;
			}
		}
	}

	if (ctx.game.isKeyPressed(GLFW_KEY_C)) {
		ctx.plane.fluids().replaceInTile(placePos_, ctx.world.getFluid("core::water").id);
	} else if (ctx.game.isKeyPressed(GLFW_KEY_V)) {
		ctx.plane.fluids().replaceInTile(placePos_, ctx.world.getFluid("core::oil").id);
	}

	// Toggle inventory
	if (ctx.game.wasKeyPressed(GLFW_KEY_E)) {
		showInventory_ = !showInventory_;
	}

	// Break block
	if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_LEFT)) {
		onLeftClick(ctx);
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
	if (ctx.game.isKeyPressed(GLFW_KEY_A) || ctx.game.isKeyPressed(GLFW_KEY_LEFT)) {
		runDirection -= 1;
	}
	if (ctx.game.isKeyPressed(GLFW_KEY_D) || ctx.game.isKeyPressed(GLFW_KEY_RIGHT)) {
		runDirection += 1;
	}

	// Handle ladder climb
	if (inLadder) {
		if (
			ctx.game.isKeyPressed(GLFW_KEY_W) ||
			ctx.game.isKeyPressed(GLFW_KEY_UP)) {
			physicsBody_.force += Swan::Vec2{0, -LADDER_CLIMB_FORCE};
		}
		else if (
			ctx.game.isKeyPressed(GLFW_KEY_S) ||
			ctx.game.isKeyPressed(GLFW_KEY_DOWN)) {
			physicsBody_.force += Swan::Vec2{0, LADDER_CLIMB_FORCE};
		}

		if (physicsBody_.vel.y > LADDER_MAX_VEL) {
			physicsBody_.vel.y = LADDER_MAX_VEL;
		}
		else if (physicsBody_.vel.y < -LADDER_MAX_VEL) {
			physicsBody_.vel.y = -LADDER_MAX_VEL;
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
	runningAnimation_.setInterval(sprinting_ ? 0.07 : 0.1);

	// If we hit the ground, override the desired state to be landing
	if (physicsBody_.onGround && (oldState == State::FALLING || oldState == State::JUMPING)) {
		state_ = State::LANDING;
	}

	// Don't switch away from landing unless it's done!
	if (oldState == State::LANDING && !landingAnimation_.done()) {
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
			currentAnimation_ = &idleAnimation_;
			break;

		case State::RUNNING:
			currentAnimation_ = &runningAnimation_;
			break;

		case State::FALLING:
			currentAnimation_ = &fallingAnimation_;
			break;

		case State::JUMPING:
			currentAnimation_ = &jumpingAnimation_;
			break;

		case State::LANDING:
			currentAnimation_ = &landingAnimation_;
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
{}

void PlayerEntity::serialize(
	const Swan::Context &ctx, Proto::Builder w)
{
	physicsBody_.serialize(w.initBody());
	inventory_.serialize(w.initInventory());
	heldStack_.serialize(w.initHeldStack());
}

void PlayerEntity::deserialize(
	const Swan::Context &ctx, Proto::Reader r)
{
	physicsBody_.deserialize(r.getBody());
	inventory_.deserialize(ctx, r.getInventory());
	heldStack_.deserialize(ctx, r.getHeldStack());
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

	Swan::InventorySlot slot = inventory_.slot(selectedInventorySlot_);
	Swan::ItemStack stack = slot.get();

	if (stack.empty()) {
		return;
	}

	Swan::Item &item = *stack.item();

	if (item.onActivate) {
		Swan::Vec2 origin = physicsBody_.body.topMid().add(0, 0.25);
		Swan::Vec2 mouse = ctx.game.getMousePos();
		item.onActivate(ctx, slot, origin, (mouse - origin).norm());
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

	if (slot.get().empty()) {
		return;
	}

	if (!ctx.plane.placeTile(placePos_, item.tile->id)) {
		return;
	}

	interactTimer_ = 0.2;
	slot.remove(1);
}

void PlayerEntity::craft(const Swan::Recipe &recipe)
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
	if (!stack.empty()) {
		Swan::info
			<< "Attempted to craft " << recipe.output.item->name
			<< ", but there wasn't enough space in the inventory to put the result";
		inventory_ = backup;
	}
}

void PlayerEntity::dropItem(const Swan::Context &ctx)
{
	auto &stack = inventory_.content[selectedInventorySlot_];

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

}
