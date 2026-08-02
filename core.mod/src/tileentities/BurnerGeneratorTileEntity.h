#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"
#include "swan/ItemStack.h"
#include "traits/PowerBufferTrait.h"
#include "traits/PowerNodeTrait.h"

namespace CoreMod {

class BurnerGeneratorTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait,
	public Swan::InventoryTrait,
	public PowerBufferTrait,
	public PowerNodeTrait {
public:
	using Proto = proto::BurnerGeneratorTileEntity;

	BurnerGeneratorTileEntity(Swan::Ctx &);

	Swan::TileEntity &get(Swan::TileEntityTrait::Tag) override
	{ return tileEntity_; }

	Swan::Inventory &get(Swan::InventoryTrait::Tag) override
	{ return inventory_; }

	PowerBuffer &get(PowerBufferTrait::Tag) override
	{ return power_; }

	PowerNode &get(PowerNodeTrait::Tag) override
	{ return powerNode_; }

	void tick2(Swan::Ctx &ctx, float dt) override;
	void onDespawn(Swan::Ctx &ctx) override;

	void drawDebug(Swan::Ctx &ctx) override;

	void serialize(Swan::Ctx &ctx, Proto::Builder w);
	void deserialize(Swan::Ctx &ctx, Proto::Reader r);

private:
	class Inventory: public Swan::Inventory {
	public:
		Swan::ItemStack take(int slot) override;
		Swan::ItemStack set(int slot, Swan::ItemStack stack) override;
		Swan::ItemStack insertInto(Swan::ItemStack stack, int from, int to) override;

		std::span<const Swan::ItemStack> content() const override
		{ return {&stack_, 1}; }

		Swan::ItemStack stack_;
	};

	Swan::TileEntity tileEntity_;
	Inventory inventory_;
	PowerBuffer power_;
	PowerNode powerNode_{{4.5/32, 27.5/32}};

	float currentBurnTime_ = 0;
	float burnRate_ = 0;
};

}
