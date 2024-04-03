#include "PlayerEntity.h"

#include <algorithm>
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
		physicsBody_.body.pos - Swan::Vec2{0.6, 0.1}));

	rnd.drawRect({Swan::Vec2(placePos_).add(0.1, 0.1), {0.8, 0.8}});
	rnd.drawRect({breakPos_, {1, 1}});
}

void PlayerEntity::ui(const Swan::Context &ctx)
{
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
	if (interactTimer_ >= 0) {
		interactTimer_ -= dt;
	}

	State oldState = state_;

	state_ = State::IDLE;

	Swan::Vec2 facePos = physicsBody_.body.topMid() + Swan::Vec2{0, 0.3};
	Swan::Vec2 mousePos = ctx.game.getMousePos();
	auto lookVector = mousePos - facePos;
	auto raycast = ctx.plane.raycast(
		facePos, lookVector, std::min(lookVector.length(), 6.0f));
	breakPos_ = raycast.pos;
	placePos_ = raycast.pos + raycast.face;

	jumpTimer_.tick(dt);

	// Figure out what tile we're in
	auto midTilePos = Swan::TilePos{
		(int)floor(physicsBody_.body.midX()),
		(int)floor(physicsBody_.body.bottom() - 0.1),
	};
	auto &midTile = ctx.plane.getTile(midTilePos);

	// Figure out what tile is below us
	auto belowTilePos = Swan::TilePos{
		(int)floor(physicsBody_.body.midX()),
		(int)floor(physicsBody_.body.bottom() + 0.1),
	};
	auto &belowTile = ctx.plane.getTile(belowTilePos);

	bool inLadder = dynamic_cast<LadderTileTrait *>(midTile.traits.get());

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
		if (ctx.game.isKeyPressed(GLFW_KEY_W) || ctx.game.isKeyPressed(GLFW_KEY_UP)) {
			physicsBody_.force += Swan::Vec2{0, -LADDER_CLIMB_FORCE};
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
	if (!physicsBody_.onGround && (!jumpPressed || physicsBody_.vel.y > 0)) {
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
		auto *sound = belowTile.stepSounds[stepIndex_];
		ctx.game.playSound(sound);
		stepIndex_ = (stepIndex_ + 1) % 2;
		stepTimer_ = 0.2;
	}
	else {
		stepTimer_ = 0.1;
	}

	// Collide with stuff
	for (auto &c: ctx.plane.getCollidingEntities(physicsBody_.body)) {
		auto *entity = c.ref.get();

		// Pick it up if it's an item stack, and don't collide
		auto *itemStackEnt = dynamic_cast<ItemStackEntity *>(entity);
		if (itemStackEnt) {
			// Don't pick up immediately
			if (itemStackEnt->lifetime_ < 0.2) {
				continue;
			}

			Swan::ItemStack stack{itemStackEnt->item(), 1};
			stack = inventory_.insert(stack);
			if (stack.empty()) {
				ctx.plane.despawnEntity(c.ref);
				ctx.game.playSound(snapSound_);
			}
			continue;
		}

		physicsBody_.collideWith(c.body);

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

	physicsBody_.standardForces();
	physicsBody_.update(ctx, dt);
}

void PlayerEntity::tick(const Swan::Context &ctx, float dt)
{}

void PlayerEntity::serialize(
	const Swan::Context &ctx, MsgStream::MapBuilder &w)
{
	w.writeString("body");
	physicsBody_.serialize(w);
	w.writeString("inventory");
	inventory_.serialize(w);
}

void PlayerEntity::deserialize(
	const Swan::Context &ctx, MsgStream::MapParser &r)
{
	std::string key;

	while (r.nextKey(key)) {
		if (key == "body") {
			physicsBody_.deserialize(r);
		}
		else if (key == "inventory") {
			inventory_.deserialize(ctx, r);
		}
		else {
			r.skipNext();
		}
	}
}

void PlayerEntity::onLeftClick(const Swan::Context &ctx)
{
	if (interactTimer_ > 0) {
		return;
	}

	breakTileAndDropItem(ctx, breakPos_);
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

	Swan::Tile &placeTile = ctx.plane.getTile(placePos_);
	if (placeTile.name != "@::air") {
		return;
	}

	if (
		item.tile->isSolid &&
		!ctx.plane.getEntitiesInTile(placePos_).empty()) {
		return;
	}

	interactTimer_ = 0.2;

	if (slot.remove(1).empty()) {
		Swan::info << "Not placing tile because stack.remove(1) is empty";
	}

	ctx.plane.setTileID(placePos_, item.tile->id);
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
	ctx.plane.spawnEntity<ItemStackEntity>(pos, vel, removed.item());
}

}
