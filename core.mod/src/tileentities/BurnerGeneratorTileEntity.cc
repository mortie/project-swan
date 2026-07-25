#include "BurnerGeneratorTileEntity.h"
#include "swan/ItemStack.h"
#include "traits/items.h"
#include <sys/resource.h>

namespace CoreMod {

static constexpr Ampere POWER_AMPERE = 0.5;
static constexpr Volt POWER_VOLTAGE = 12;

void BurnerGeneratorTileEntity::tick(Swan::Ctx &ctx, float dt)
{
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

	float frac = power_.chargeUp(POWER_AMPERE, POWER_VOLTAGE);
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
	ImGui::Text("Burning: %d", currentBurnTime_ > 0);
	ImGui::Text("Voltage: %f", power_.voltage());
	ImGui::Text("Burn rate: %f", burnRate_);
	ImGui::Text("Burn time: %f", currentBurnTime_);
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
