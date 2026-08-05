#include "BonfireTileEntity.h"
#include "entities/ItemStackEntity.h"
#include "world/util.h"
#include "data/sounds.h"

namespace CoreMod {

void BonfireTileEntity::update(Swan::Ctx &ctx, float dt)
{
	for (auto &found: ctx.plane.entities().getInTile(tileEntity_.pos)) {
		auto stack = dynamic_cast<ItemStackEntity *>(found.ref.get());
		if (!stack) {
			continue;
		}

		if (stack->lifetime_ < 0.1) {
			continue;
		}

		auto &body = stack->get(Swan::PhysicsBodyTrait::Tag{});
		auto vel = body.velocity();
		vel.y *= 0.5;
		body.applyForce(vel * -500);
	}
}

void BonfireTileEntity::tick(Swan::Ctx &ctx, float dt)
{
	tickBonfire(ctx, dt);
	tickCrucible(ctx, dt);
}

void BonfireTileEntity::tickBonfire(Swan::Ctx &ctx, float dt)
{
	Swan::Vec2 tileCenter = {tileEntity_.pos.x + 0.5f, tileEntity_.pos.y + 0.5f};

	// Spawn smoke particles always
	auto layer = Swan::random() % 3
		? Cygnet::RenderLayer::BEHIND
		: Cygnet::RenderLayer::NORMAL;
	ctx.game.spawnParticle(layer, {
		.pos = tileCenter.add((Swan::randfloat() - 0.5f) * 0.2f, 0.2f),
		.vel = {(Swan::randfloat() - 0.5f) * 1, -(Swan::randfloat() * 0.5f + 0.6f)},
		.size = {1.0f / 16, 1.0f / 16},
		.color = {0.3f, 0.3f, 0.3f, Swan::randfloat() * 0.2f + 0.75f},
		.lifetime = Swan::randfloat() + 0.2f,
		.weight = 0.05,
	});

	// Progress existing burns
	for (size_t i = 0; i < ongoing_.size();) {
		auto &burn = ongoing_[i];

		// Show fire particles on burning items,
		// or abort burns if the items have disappeared
		bool ok = true;
		for (auto &ent: burn.inputs) {
			auto *body = ent.getBody();
			if (!body) {
				ok = false;
				break;
			}

			auto center = body->center();
			if ((tileCenter - center).squareLength() > 1.2 * 1.2) {
				ok = false;
				break;
			}

			if (Swan::random() % 2 != 0) {
				continue;
			}

			float r = 0.5f + (Swan::randfloat() * 0.4f);
			float g = std::min(0.1f + (Swan::randfloat() * 0.8f), r);
			ctx.game.spawnParticle({
				.pos = center.add(-0.5f / 16, (Swan::randfloat() - 0.5f) * 0.2f),
				.vel = {(Swan::randfloat() - 0.5f) * 2, -(Swan::randfloat() * 0.5f + 1)},
				.size = {1.0f / 16, 1.0f / 16},
				.color = {r, g, 0},
				.lifetime = 0.2f + Swan::randfloat() * 0.3f,
				.weight = 0.3,
			});
		}

		// Abort burn
		if (!ok) {
			ongoing_[i] = ongoing_.back();
			ongoing_.pop_back();
			continue;
		}

		// Complete burn
		burn.timer -= dt;
		if (burn.timer <= 0) {
			for (auto &ent: burn.inputs) {
				ctx.plane.entities().despawn(ent);
			}

			float dir = Swan::randfloat() > 0.5 ? 1 : -1;
			for (int j = 0; j < burn.output.count(); ++j) {
				ctx.plane.entities().spawn<ItemStackEntity>(
					tileCenter,
					Swan::Vec2{(Swan::randfloat() * 1 + 2) * dir, -7},
					burn.output.item());
				dir *= -1;
			}

			ongoing_[i] = ongoing_.back();
			ongoing_.pop_back();
			continue;
		}

		i += 1;
	}

	// Find usable items
	std::unordered_map<Swan::Item *, std::vector<Swan::EntityRef>> items;
	for (auto &found: ctx.plane.entities().getInTile(tileEntity_.pos)) {
		auto stack = dynamic_cast<ItemStackEntity *>(found.ref.get());
		if (!stack) {
			continue;
		}

		bool used = false;
		for (auto &burn: ongoing_) {
			for (auto &input: burn.inputs) {
				if (input == found.ref) {
					used = true;
					break;
				}
			}
			if (used) {
				break;
			}
		}

		if (used) {
			continue;
		}

		items[stack->item()].push_back(found.ref);
	}

	// Find eligible recipes
	for (auto &recipe: ctx.world.getRecipes("core::burning")) {
		bool ok = true;
		for (auto &input: recipe.inputs) {
			auto it = items.find(input.item());
			if (it == items.end() || int(it->second.size()) < input.count()) {
				ok = false;
				break;
			}
		}

		if (!ok) {
			continue;
		}

		ongoing_.push_back({
			.inputs = {},
			.output = recipe.output,
			.timer = 2,
		});
		auto &burn = ongoing_.back();

		for (auto &input: recipe.inputs) {
			auto &ents = items[input.item()];
			for (int i = 0; i < input.count(); ++i) {
				burn.inputs.push_back(ents.back());
				ents.pop_back();
			}
		}
	}
}

void BonfireTileEntity::tickCrucible(Swan::Ctx &ctx, float dt)
{
	if (!crucible_.progress) {
		return;
	}

	constexpr float temperature = 1;
	Swan::Vec2 center = Swan::tileCenter(tileEntity_.pos).add(0, -0.05);
	float particleCount = temperature * Swan::randfloat() * 2;
	for (int i = 0; i < particleCount; ++i) {
		float r = 0.5f + (Swan::randfloat() * 0.4f);
		float g = std::min(0.1f + (Swan::randfloat() * 0.8f), r);
		ctx.game.spawnParticle({
			.pos = center.add(
				-0.5f / 16 + (Swan::randfloat() - 0.5) * 0.5 * temperature, -0.2f),
			.vel = {
				(Swan::randfloat() - 0.5f) * 2,
				-(Swan::randfloat() * 0.5f + temperature),
			},
			.size = {1.0f / 16, 1.0f / 16},
			.color = {r, g, 0},
			.lifetime = (0.2f + Swan::randfloat() * 0.3f) * temperature,
			.weight = 0.3,
		});
	}

	crucible_.progress->timer -= dt * temperature;
	if (crucible_.progress->timer <= 0) {
		float dir = Swan::randfloat() > 0.5 ? 1 : -1;
		for (int i = 0; i < crucible_.progress->output.count(); ++i) {
			ctx.plane.entities().spawn<ItemStackEntity>(
				center,
				Swan::Vec2{(Swan::randfloat() * 2 + 1) * dir, -7},
				crucible_.progress->output.item());
			dir *= -1;
		}
		crucible_.items.clear();
		crucible_.itemCounts.clear();
		crucible_.progress.reset();
	}
}

void BonfireTileEntity::serialize(Swan::Ctx &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w.initTileEntity());

	auto burns = w.initOngoing(ongoing_.size());
	for (size_t i = 0; i < ongoing_.size(); ++i) {
		auto b = burns[i];
		auto &burn = ongoing_[i];

		auto inputs = b.initInputs(burn.inputs.size());
		for (size_t j = 0; j < burn.inputs.size(); ++j) {
			burn.inputs[j].serialize(inputs[j]);
		}

		burn.output.serialize(b.initOutput());
		b.setTimer(burn.timer);
	}
}

void BonfireTileEntity::deserialize(Swan::Ctx &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());

	ongoing_.clear();
	auto ongoing = r.getOngoing();
	for (auto b: ongoing) {
		auto inputs = b.getInputs();

		ongoing_.push_back({
			.inputs = {},
			.output = {},
			.timer = b.getTimer(),
		});

		auto &burn = ongoing_.back();
		burn.inputs.reserve(inputs.size());
		burn.output.deserialize(ctx, b.getOutput());

		for (auto input: inputs) {
			burn.inputs.push_back({});
			burn.inputs.back().deserialize(ctx, input);
		}
	}
}

void BonfireTileEntity::activateCrucible(Swan::Ctx &ctx, Swan::ItemStack &stack)
{
	if (crucible_.progress) {
		return;
	}

	if (stack.empty()) {
		evacuateCrucible(ctx);
		return;
	}

	bool compatible = false;
	for (auto &recipe: ctx.world.getRecipes("core::smelting")) {
		bool complete = true;
		for (auto &input: recipe.inputs) {
			int count = 0;
			if (input.item() == stack.item()) {
				count += 1;
			}
			auto it = crucible_.itemCounts.find(input.item());
			if (it != crucible_.itemCounts.end()) {
				count += it->second;
			}

			if (input.item() == stack.item() && count <= input.count()) {
				compatible = true;
			}

			if (count < input.count()) {
				complete = false;
			}
		}

		if (complete) {
			ctx.game.playSound(sounds::misc__snap);
			auto it = stack.remove(1);
			crucible_.items.push_back(it.item());
			crucible_.progress = Crucible::Progress {
				.timer = 4,
				.output = recipe.output,
			};
			return;
		}
	}

	if (!compatible) {
		return;
	}

	ctx.game.playSound(sounds::misc__snap);
	auto it = stack.remove(1);
	crucible_.items.push_back(it.item());
	crucible_.itemCounts[it.item()] += 1;
}

void BonfireTileEntity::evacuateCrucible(Swan::Ctx &ctx)
{
	for (auto &item: crucible_.items) {
		dropItem(ctx, tileEntity_.pos, *item);
	}

	crucible_.items.clear();
	crucible_.itemCounts.clear();
	crucible_.progress.reset();
}

}
