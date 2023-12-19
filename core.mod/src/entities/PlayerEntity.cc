#include "PlayerEntity.h"

#include <cmath>
#include <compare>
#include <imgui/imgui.h>

#include "ItemStackEntity.h"

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

void PlayerEntity::ui() {
	ImGui::Begin("Inventory");

	auto &selectedStack = inventory_.content[selectedInventorySlot_];
	if (selectedStack.empty()) {
		ImGui::Text("Selected: [%d]: Empty", selectedInventorySlot_);
	} else {
		ImGui::Text("Selected: [%d]: %d x %s", selectedInventorySlot_,
			selectedStack.count(), selectedStack.item()->name.c_str());
	}

	for (size_t i = 0; i < inventory_.content.size(); ++i) {
		auto &stack = inventory_.content[i];
		if (stack.empty()) {
			continue;
		}

		ImGui::Text("%zu: %d x %s", i, stack.count(),
			stack.item()->name.c_str());
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
	if (ctx.game.isMousePressed(GLFW_MOUSE_BUTTON_RIGHT) && placeTimer_.periodic(0.50)) {
		if (ctx.plane.getTileID(mouseTile_) == ctx.world.getTileID("@::air")) {
			Swan::ItemStack stack = inventory_.content[selectedInventorySlot_].remove(1);
			if (!stack.empty()) {
				ctx.plane.setTile(mouseTile_, stack.item()->name);
			}
		}
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

		// Pick it up if it's an item stack
		auto *itemStackEnt = dynamic_cast<ItemStackEntity *>(entity);
		if (itemStackEnt) {
			Swan::ItemStack stack{itemStackEnt->item(), 1};
			stack = inventory_.insert(0, stack);
			if (stack.empty()) {
				ctx.plane.despawnEntity(c.ref);
			}
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
