#include "PlayerEntity.h"

#include <imgui/imgui.h>
#include <unordered_map>

#include "ItemStackEntity.h"
#include "swan-common/Vector2.h"

PlayerEntity::PlayerEntity(const Swan::Context &ctx, Swan::Vec2 pos):
		PlayerEntity(ctx) {
	physicsBody_.body.pos = pos;
}

PlayerEntity::PlayerEntity(const Swan::Context &ctx, const PackObject &obj):
		PlayerEntity(ctx) {
	deserialize(ctx, obj);
}

void PlayerEntity::draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) {
	Cygnet::Mat3gf mat;

	// Currently, there is no sprite for running left.
	// Running left is just running right but flipped.
	if (state_ == State::RUNNING_L) {
		mat.translate({-0.5, 0}).scale({-1, 1}).translate({0.5, 0});
	}

	currentAnimation_->draw(rnd, mat.translate(
		physicsBody_.body.pos - Swan::Vec2{0.2, 0.1}));

	rnd.drawRect({mouseTile_, {1, 1}});
}

void PlayerEntity::ui(const Swan::Context &ctx) {
	// This whole method is stupid inefficient and does a bunch of memory allocation every frame.
	// TODO: fix.

	ImGui::Begin("Inventory");
	auto &selectedStack = inventory_.content[selectedInventorySlot_];
	if (selectedStack.empty()) {
		ImGui::Text("Selected: [%d]: Empty", selectedInventorySlot_);
	} else {
		ImGui::Text("Selected: [%d]: %d x %s", selectedInventorySlot_,
			selectedStack.count(), selectedStack.item()->name.c_str());
	}

	std::unordered_map<Swan::Item *, int> itemCounts;
	for (size_t i = 0; i < inventory_.content.size(); ++i) {
		auto &stack = inventory_.content[i];
		if (stack.empty()) {
			continue;
		}

		itemCounts[stack.item()] += stack.count();

		ImGui::Text("%zu: %d x %s", i, stack.count(),
			stack.item()->name.c_str());
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
		for (const auto &input: recipe.inputs) {
			if (text != "") {
				text += ", ";
			}

			text += std::to_string(input.count);
			text += ' ';
			text += input.item->name;
		}

		text += " => ";
		text += std::to_string(recipe.output.count);
		text += ' ';
		text += recipe.output.item->name;
		ImGui::Text("%s", text.c_str());

		if (ImGui::Button("Craft")) {
			craft(recipe);
		}
	}
	ImGui::End();
}

void PlayerEntity::update(const Swan::Context &ctx, float dt) {
	State oldState = state_;
	state_ = State::IDLE;

	mouseTile_ = ctx.game.getMouseTile();
	jumpTimer_.tick(dt);
	placeTimer_.tick(dt);

	// Select item slots
	if (ctx.game.wasKeyPressed(GLFW_KEY_1)) {
		selectedInventorySlot_ = 0;
	} else if (ctx.game.wasKeyPressed(GLFW_KEY_2)) {
		selectedInventorySlot_ = 1;
	} else if (ctx.game.wasKeyPressed(GLFW_KEY_3)) {
		selectedInventorySlot_ = 2;
	} else if (ctx.game.wasKeyPressed(GLFW_KEY_4)) {
		selectedInventorySlot_ = 3;
	} else if (ctx.game.wasKeyPressed(GLFW_KEY_5)) {
		selectedInventorySlot_ = 4;
	} else if (ctx.game.wasKeyPressed(GLFW_KEY_6)) {
		selectedInventorySlot_ = 5;
	} else if (ctx.game.wasKeyPressed(GLFW_KEY_7)) {
		selectedInventorySlot_ = 6;
	} else if (ctx.game.wasKeyPressed(GLFW_KEY_8)) {
		selectedInventorySlot_ = 7;
	} else if (ctx.game.wasKeyPressed(GLFW_KEY_9)) {
		selectedInventorySlot_ = 8;
	} else if (ctx.game.wasKeyPressed(GLFW_KEY_0)) {
		selectedInventorySlot_ = 9;
	}

	// Break block
	if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_LEFT))
		ctx.plane.breakTile(mouseTile_);

	// Place block
	if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT)) {
		placeTile(ctx);
	}

	// Drop item
	if (ctx.game.wasKeyPressed(GLFW_KEY_Q)) {
		dropItem(ctx);
	}

	// Move left
	if (ctx.game.isKeyPressed(GLFW_KEY_A) || ctx.game.isKeyPressed(GLFW_KEY_LEFT)) {
		physicsBody_.force += Swan::Vec2(-MOVE_FORCE, 0);
		state_ = State::RUNNING_L;
	}

	// Move right
	if (ctx.game.isKeyPressed(GLFW_KEY_D) || ctx.game.isKeyPressed(GLFW_KEY_RIGHT)) {
		physicsBody_.force += Swan::Vec2(MOVE_FORCE, 0);
		if (state_ == State::RUNNING_L)
			state_ = State::IDLE;
		else
			state_ = State::RUNNING_R;
	}

	bool jumpPressed = ctx.game.isKeyPressed(GLFW_KEY_SPACE);

	// Jump
	if (physicsBody_.onGround && jumpPressed && jumpTimer_.periodic(0.5)) {
		physicsBody_.vel.y = -JUMP_VEL;
	}

	// Fall down faster than we went up
	if (!physicsBody_.onGround && (!jumpPressed || physicsBody_.vel.y > 0))
		physicsBody_.force += Swan::Vec2(0, DOWN_FORCE);

	if (state_ != oldState) {
		switch (state_) {
		case State::IDLE:
			currentAnimation_ = &idleAnimation_;
			break;
		case State::RUNNING_L:
		case State::RUNNING_R:
			currentAnimation_ = &runningAnimation_;
			break;
		}
		currentAnimation_->reset();
	}
	currentAnimation_->tick(dt);

	if (invincibleTimer_ >= 0) {
		invincibleTimer_ -= dt;
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
			stack = inventory_.insert(0, stack);
			if (stack.empty()) {
				ctx.plane.despawnEntity(c.ref);
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
			} else {
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

void PlayerEntity::tick(const Swan::Context &ctx, float dt) {
}

void PlayerEntity::deserialize(const Swan::Context &ctx, const PackObject &obj) {
	//body_.deserialize(obj["body"]);
}

Swan::Entity::PackObject PlayerEntity::serialize(const Swan::Context &ctx, msgpack::zone &zone) {
	return {};
	/*
	return Swan::MsgPackObject{
		{ "body", body_.serialize(w) },
	};
	*/
}

void PlayerEntity::placeTile(const Swan::Context &ctx) {
	Swan::ItemStack &stack = inventory_.content[selectedInventorySlot_];
	if (stack.empty()) {
		return;
	}

	Swan::Item &item = *stack.item();
	if (!item.tile) {
		return;
	}

	Swan::Tile &tileAtMouse = ctx.plane.getTile(mouseTile_);
	if (tileAtMouse.name != "@::air") {
		return;
	}

	if (!ctx.plane.getEntitiesInTile(mouseTile_).empty()) {
		return;
	}

	if (!placeTimer_.periodic(0.50)) {
		return;
	}

	if (stack.remove(1).empty()) {
		Swan::info << "Not placing tile because stack.remove(1) is empty";
	}

	ctx.plane.setTile(mouseTile_, item.tile.value());
}

void PlayerEntity::craft(const Swan::Recipe &recipe) {
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
			Swan::info << "Attempted to craft " << recipe.output.item
				<< ", but missing " << needed << ' ' << input.item;
			inventory_ = backup;
			return;
		}
	}

	Swan::ItemStack stack(recipe.output.item, recipe.output.count);
	stack = inventory_.insert(0, stack);
	if (!stack.empty()) {
		Swan::info << "Attempted to craft " << recipe.output.item
			<< ", but there wasn't enough space in the inventory to put the result";
		inventory_ = backup;
	}
}

void PlayerEntity::dropItem(const Swan::Context &ctx) {
	auto &stack = inventory_.content[selectedInventorySlot_];
	if (stack.empty()) {
		return;
	}

	auto pos = physicsBody_.body.topMid() + Swan::Vec2{0, 0.5};
	auto direction = (Swan::Vec2{float(mouseTile_.x), float(mouseTile_.y)} - pos).norm();
	auto vel = direction * 20.0;

	auto removed = stack.remove(1);
	ctx.plane.spawnEntity<ItemStackEntity>(pos, vel, removed.item());
}
