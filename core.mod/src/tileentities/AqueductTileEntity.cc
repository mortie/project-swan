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
}

void AqueductTileEntity::tick(Swan::Ctx &ctx, float dt)
{
	constexpr float VISCOSITY = 0.5;

	if (content_.level < 0.01) {
		content_.level = 0;
		content_.fluid = nullptr;
	}

	AqueductTileEntity *left = left_.as<AqueductTileEntity>();
	AqueductTileEntity *right = right_.as<AqueductTileEntity>();

	velLeft_ *= 0.9;
	velRight_ *= 0.9;

	float leftLevel = left ? left->content_.level : 0;
	bool moveLeft =
		(!left || left->content_.fluid == content_.fluid) &&
		leftLevel < content_.level;
	if (moveLeft) {
		float delta = content_.level - left->content_.level;
		velLeft_ += delta * VISCOSITY;
	}

	float rightLevel = right ? right->content_.level : 0;
	bool moveRight =
		(!right || right->content_.fluid == content_.fluid) &&
		rightLevel < content_.level;
	if (moveRight) {
		float delta = content_.level - right->content_.level;
		velRight_ += delta * VISCOSITY;
	}
}

void AqueductTileEntity::tick2(Swan::Ctx &ctx, float dt)
{
	if (!content_.fluid || (velLeft_ < 0.01 && velRight_ < 0.01)) {
		return;
	}

	AqueductTileEntity *left = left_.as<AqueductTileEntity>();
	AqueductTileEntity *right = right_.as<AqueductTileEntity>();

	float leftFrac = velLeft_ / (velLeft_ + velRight_);
	float sum = std::min(velLeft_ + velRight_, content_.level);
	float outLeft = sum * leftFrac;
	float outRight = sum * (1 - leftFrac);

	if (left) {
		left->content_.level += outLeft;
		left->content_.fluid = content_.fluid;
		content_.level -= outLeft;
	} else if (outLeft > 0.1) {
		content_.level -= 0.1;
		ctx.plane.fluids().spawnFluidParticle(
			tileEntity_.pos.as<float>().add(-0.5, 0.5),
			content_.fluid->id);
	}

	if (right) {
		right->content_.level += outRight;
		left->content_.fluid = content_.fluid;
	} else if (outRight > 0.1) {
		content_.level -= 0.1;
		ctx.plane.fluids().spawnFluidParticle(
			tileEntity_.pos.as<float>().add(0.5, 0.5),
			content_.fluid->id);
	}
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
	onTileUpdate(ctx);
}

}
