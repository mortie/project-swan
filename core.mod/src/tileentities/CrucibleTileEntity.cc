#include "CrucibleTileEntity.h"
#include "entities/ItemStackEntity.h"
#include "world/util.h"

namespace CoreMod {

void CrucibleTileEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	if (temperature_ > 0) {
		rnd.drawTileSprite({
			.transform = Cygnet::Mat3gf{}
				.translate(tileEntity_.pos.as<float>()),
			.sprite = overlaySprite_,
			.opacity = temperature_ * 0.7f,
		});
	}

	if (drawSupports_) {
		rnd.drawSprite(Cygnet::RenderLayer::BEHIND, {
			.transform = Cygnet::Mat3gf{}
				.translate(tileEntity_.pos.as<float>().add(0, 1)),
			.sprite = supportSprite_,
		});
	}
}

void CrucibleTileEntity::tick(Swan::Ctx &ctx, float dt)
{
	constexpr float delta = 0.1;

	if (temperature_ > targetTemperature_) {
		temperature_ -= delta * dt;
		if (temperature_ < targetTemperature_) {
			temperature_ = targetTemperature_;
		}
	}
	else if (temperature_ < targetTemperature_) {
		temperature_ += delta * dt;
		if (temperature_ > targetTemperature_) {
			temperature_ = targetTemperature_;
		}
	}

	if (!progress_) {
		return;
	}

	if (temperature_ <= 0) {
		progress_.reset();
		for (auto &item: items_) {
			dropItem(ctx, tileEntity_.pos, *item);
		}
		items_.clear();
		itemCounts_.clear();
		return;
	}

	Swan::Vec2 center = tileEntity_.pos.as<float>().add(0.5, 0.5);
	float particleCount = temperature_ * Swan::randfloat() * 2;
	for (int i = 0; i < particleCount; ++i) {
		float r = 0.5f + (Swan::randfloat() * 0.4f);
		float g = std::min(0.1f + (Swan::randfloat() * 0.8f), r);
		ctx.game.spawnParticle({
			.pos = center.add(
				-0.5f / 16 + (Swan::randfloat() - 0.5) * 0.5 * temperature_, -0.2f),
			.vel = {
				(Swan::randfloat() - 0.5f) * 2,
				-(Swan::randfloat() * 0.5f + temperature_),
			},
			.size = {1.0f / 16, 1.0f / 16},
			.color = {r, g, 0},
			.lifetime = (0.2f + Swan::randfloat() * 0.3f) * temperature_,
			.weight = 0.3,
		});
	}

	progress_->timer -= dt * temperature_;
	float dir = Swan::randfloat() > 0.5 ? 1 : -1;
	if (progress_->timer <= 0) {
		for (int i = 0; i < progress_->output.count(); ++i) {
			ctx.plane.entities().spawn<ItemStackEntity>(
				center,
				Swan::Vec2{(Swan::randfloat() * 2 + 1) * dir, -7},
				progress_->output.item());
			dir *= -1;
		}
		progress_.reset();
		items_.clear();
		itemCounts_.clear();
	}
}

void CrucibleTileEntity::serialize(Swan::Ctx &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w.initTileEntity());
	w.setDrawSupports(drawSupports_);
	w.setTemperature(temperature_);
	w.setTargetTemperature(targetTemperature_);
	auto items = w.initItems(items_.size());
	for (size_t i = 0; i < items_.size(); ++i) {
		items.set(i, items_[i]->name);
	}

	if (progress_) {
		auto progress = w.initProgress();
		progress.setTimer(progress_->timer);
		progress_->output.serialize(progress.initOutput());
	}
}

void CrucibleTileEntity::deserialize(Swan::Ctx &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());
	drawSupports_ = r.getDrawSupports();
	temperature_ = r.getTemperature();
	targetTemperature_ = r.getTargetTemperature();
	auto items = r.getItems();
	items_.clear();
	items_.reserve(items.size());
	itemCounts_.clear();
	for (auto item: items) {
		items_.push_back(&ctx.world.getItem(item.cStr()));
		itemCounts_[items_.back()] += 1;
	}

	if (r.hasProgress()) {
		auto progress = r.getProgress();
		progress_ = {};
		progress_->timer = progress.getTimer();
		progress_->output.deserialize(ctx, progress.getOutput());
	} else {
		progress_.reset();
	}
}

void CrucibleTileEntity::onDespawn(Swan::Ctx &ctx)
{
	for (auto &item: items_) {
		dropItem(ctx, tileEntity_.pos, *item);
	}
}

void CrucibleTileEntity::activate(Swan::Ctx &ctx, Swan::ItemStack &stack)
{
	if (progress_) {
		return;
	}

	if (stack.empty()) {
		for (auto &item: items_) {
			dropItem(ctx, tileEntity_.pos, *item);
		}
		items_.clear();
		itemCounts_.clear();
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
			auto it = itemCounts_.find(input.item());
			if (it != itemCounts_.end()) {
				count += it->second;
			}

			if (input.item() == stack.item() && count <= input.count()) {
				Swan::info << "Compatible with recipe for " << recipe.output.item()->name;
				compatible = true;
			}

			if (count < input.count()) {
				complete = false;
			}
		}

		if (complete) {
			ctx.game.playSound(ctx.world.getSound("core::sounds/misc/snap"));
			auto it = stack.remove(1);
			items_.push_back(it.item());
			progress_ = Progress {
				.timer = 4,
				.output = recipe.output,
			};
			return;
		}
	}

	if (!compatible) {
		return;
	}

	ctx.game.playSound(ctx.world.getSound("core::sounds/misc/snap"));
	auto it = stack.remove(1);
	items_.push_back(it.item());
	itemCounts_[it.item()] += 1;
}

}
