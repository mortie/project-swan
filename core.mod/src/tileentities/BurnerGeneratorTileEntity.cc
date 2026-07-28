#include "BurnerGeneratorTileEntity.h"
#include "swan/ItemStack.h"
#include "traits/items.h"
#include "world/util.h"
#include <sys/resource.h>

namespace CoreMod {

static constexpr Ampere POWER_AMPERE = 0.4;
static constexpr Volt POWER_VOLTAGE = 230;
static constexpr Farad CAPACITANCE = 5e-3;

BurnerGeneratorTileEntity::BurnerGeneratorTileEntity(Swan::Ctx &ctx)
	: power_(CAPACITANCE)
{}

void BurnerGeneratorTileEntity::tick2(Swan::Ctx &ctx, float dt)
{
	power_.tick2();

	if (currentBurnTime_ <= 0) {
		burnRate_ = 0;

		auto stack = inventory_.stack_.remove(1);
		if (!stack) {
			return;
		}

		auto burnable = dynamic_cast<BurnableItemTrait *>(
			stack.item()->traits.get());
		if (!burnable) {
			return;
		}

		currentBurnTime_ = burnable->burnTime;
	}

	float frac = power_.chargeUp(POWER_AMPERE, POWER_VOLTAGE, dt);
	burnRate_ = (frac + 0.2) / 1.2;
	currentBurnTime_ -= dt * burnRate_;

	if (Swan::randfloat() <= burnRate_) {
		auto center = Swan::tileCenter(tileEntity_.pos);
		ctx.game.spawnParticle({
			.pos = center.add((Swan::randfloat() - 0.5f) * 0.2f, 0),
			.vel = {(Swan::randfloat() - 0.5f) * 1, -(Swan::randfloat() * 0.5f + 0.6f)},
			.size = {1.0f / 16, 1.0f / 16},
			.color = {0.3f, 0.3f, 0.3f, Swan::randfloat() * 0.2f + 0.75f},
			.lifetime = Swan::randfloat() + 0.2f,
			.weight = 0.05,
		});
	}
}

void BurnerGeneratorTileEntity::drawDebug(Swan::Ctx &ctx)
{
	power_.drawDebug();
	ImGui::Text("Burn rate: %.1fx", burnRate_);
	ImGui::Text("Burn time: %.1fs", currentBurnTime_);
}

void BurnerGeneratorTileEntity::onDespawn(Swan::Ctx &ctx)
{
	for (int i = 0; i < inventory_.stack_.count(); ++i) {
		dropItem(ctx, tileEntity_.pos, *inventory_.stack_.item());
	}
}

void BurnerGeneratorTileEntity::serialize(Swan::Ctx &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w.initTileEntity());
	power_.serialize(w.initPowerBuffer());
	inventory_.stack_.serialize(w.initContent());
	w.setBurnTime(currentBurnTime_);
}

void BurnerGeneratorTileEntity::deserialize(Swan::Ctx &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());
	power_.deserialize(r.getPowerBuffer());
	inventory_.stack_.deserialize(ctx, r.getContent());
	currentBurnTime_ = r.getBurnTime();
}

Swan::ItemStack BurnerGeneratorTileEntity::Inventory::take(int slot)
{
	auto stack = stack_;
	stack_ = {};
	return stack;
}

Swan::ItemStack BurnerGeneratorTileEntity::Inventory::set(
	int slot, Swan::ItemStack stack)
{
	if (stack.empty()) {
		return stack;
	}

	auto burnable = dynamic_cast<BurnableItemTrait *>(
		stack.item()->traits.get());
	if (!burnable) {
		return stack;
	}

	return stack_.insert(stack);
}

Swan::ItemStack BurnerGeneratorTileEntity::Inventory::insertInto(
	Swan::ItemStack stack,
	int from, int to)
{
	return set(0, stack);
}

}
