#include "AqueductTileEntity.h"

#include "world/aqueduct.h"

namespace CoreMod {

void AqueductTileEntity::onTileUpdate(Swan::Ctx &ctx)
{
	auto pos = tileEntity_.pos;

	auto &tile = ctx.plane.tiles().get(pos);
	auto aqueduct = dynamic_cast<AqueductTrait *>(tile.traits.get());
	if (!aqueduct) {
		Swan::warn << "Aqueduct tile entity on a tile without an aqueduct?";
		return;
	}

	if (aqueduct->connectedTo.has(Swan::Direction::LEFT)) {
		left_ = ctx.plane.entities().getTileEntity(pos.add(-1, 0));
	} else {
		left_ = {};
	}

	if (aqueduct->connectedTo.has(Swan::Direction::RIGHT)) {
		right_ = ctx.plane.entities().getTileEntity(pos.add(1, 0));
	} else {
		right_ = {};
	}

	connectedTo_ = aqueduct->connectedTo;
}

void AqueductTileEntity::draw(Swan::Ctx &ctx, Cygnet::Renderer &rnd)
{
	if (!content_.fluid) {
		return;
	}

	float startX = 0;
	float endX = 1;
	if (!left_) {
		startX += 0.25;
	}
	if (!right_) {
		endX -= 0.25;
	}

	float width = endX - startX;
	float height = content_.level * 0.25;
	rnd.drawTileParticle({
		.pos = tileEntity_.pos.as<float>().add(startX, 1 - height - 0.215),
		.size = {width, height},
		.color = content_.fluid->fg,
	});
}

void AqueductTileEntity::tick(Swan::Ctx &ctx, float dt)
{
	constexpr float VISCOSITY = 0.5;

	if (content_.level < 0.005) {
		content_.level = 0;
		content_.fluid = nullptr;
	}

	AqueductTileEntity *left = left_.as<AqueductTileEntity>();
	AqueductTileEntity *right = right_.as<AqueductTileEntity>();

	velLeft_ *= 0.9;
	velRight_ *= 0.9;
	levelSnapshot_ = content_.level;

	if (!connectedTo_.has(Swan::Direction::LEFT)) {
		velLeft_ = 0;
	} else {
		float leftLevel = left ? left->content_.level : 0;
		bool moveLeft = leftLevel < content_.level;
		if (moveLeft) {
			float delta = content_.level - leftLevel;
			velLeft_ += delta * VISCOSITY;
		}
	}

	if (!connectedTo_.has(Swan::Direction::RIGHT)) {
		velRight_ = 0;
	} else {
		float rightLevel = right ? right->content_.level : 0;
		bool moveRight = rightLevel < content_.level;
		if (moveRight) {
			float delta = content_.level - rightLevel;
			velRight_ += delta * VISCOSITY;
		}
	}
}

void AqueductTileEntity::tick2(Swan::Ctx &ctx, float dt)
{
	if (content_.level <= 0.9) {
		auto pos = tileEntity_.pos;
		if (content_.fluid) {
			bool taken = ctx.plane.fluids()
				.takeFluidFromRow(pos, 2, content_.fluid->id);
			if (taken) {
				content_.level += 0.1;
			}
		} else {
			Swan::Fluid &fluid = ctx.plane.fluids().takeAnyFromRow(pos, 2);
			if (fluid.id != Swan::World::AIR_FLUID_ID) {
				content_.fluid = &fluid;
				content_.level = 0.1;
			}
		}
	}

	if (levelSnapshot_ < 0.001 || (velLeft_ < 0.01 && velRight_ < 0.01)) {
		return;
	}

	AqueductTileEntity *left = left_.as<AqueductTileEntity>();
	AqueductTileEntity *right = right_.as<AqueductTileEntity>();

	float leftFrac = velLeft_ / (velLeft_ + velRight_);
	float sum = std::min(velLeft_ + velRight_, levelSnapshot_);
	float outLeft = sum * leftFrac;
	float outRight = sum * (1 - leftFrac);

	if (left) {
		left->content_.level += outLeft;
		left->content_.fluid = content_.fluid;
		content_.level -= outLeft;
	} else if (outLeft > 0) {
		dropAcc_ += outLeft;
		content_.level -= outLeft;

		if (dropAcc_ >= 0.1) {
			ctx.plane.fluids().spawnFluidParticle(
				tileEntity_.pos.as<float>().add(-0.25 + 0.25, 0.61),
				content_.fluid->id,
				{-5, 0.0});
			dropAcc_ -= 0.1;
		}
	}

	if (right) {
		right->content_.level += outRight;
		right->content_.fluid = content_.fluid;
		content_.level -= outRight;
	} else if (outRight > 0) {
		dropAcc_ += outRight;
		content_.level -= outRight;

		if (dropAcc_ >= 0.1) {
			ctx.plane.fluids().spawnFluidParticle(
				tileEntity_.pos.as<float>().add(1 - 0.25, 0.61),
				content_.fluid->id,
				{5, 0.0});
			dropAcc_ -= 0.1;
		}
	}
}

void AqueductTileEntity::drawDebug(Swan::Ctx &ctx)
{
	ImGui::Text("Level: %f", content_.level);
	ImGui::Text("Fluid: %s", content_.fluid ? content_.fluid->name.c_str() : "N/A");
	ImGui::Text("Connected to:");
	for (auto x: connectedTo_) {
		ImGui::Text("* %s", x.name());
	}
	ImGui::Text("Left? %d, right? %d", bool(left_), bool(right_));
}

void AqueductTileEntity::serialize(Swan::Ctx &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w.initTileEntity());
	if (content_.fluid) {
		w.setFluidType(content_.fluid->name);
		w.setFluidLevel(content_.level);
	}
}

void AqueductTileEntity::deserialize(Swan::Ctx &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());
	content_ = {};
	if (r.hasFluidType()) {
		content_.fluid = &ctx.world.getFluid(r.getFluidType().cStr());
		if (content_.fluid) {
			content_.level = r.getFluidLevel();
		}
	}

	velLeft_ = 0;
	velRight_ = 0;
	dropAcc_ = 0;
}

}
