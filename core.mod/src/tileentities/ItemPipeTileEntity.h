#pragma once

#include <swan/swan.h>

#include "core_mod.capnp.h"

namespace CoreMod {

class ItemPipeTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait,
	public Swan::InventoryTrait {
public:
	using Proto = proto::ItemPipeTileEntity;

	ItemPipeTileEntity(const Swan::Context &ctx)
	{}

	TileEntity &get(TileEntityTrait::Tag) override
	{
		return tileEntity_;
	}

	Inventory &get(InventoryTrait::Tag) override
	{
		return inbox_;
	}

	void draw(const Swan::Context &ctx, Cygnet::Renderer &rnd) override;
	void tick(const Swan::Context &ctx, float dt) override;
	void tick2(const Swan::Context &ctx, float dt) override;

	void onDespawn(const Swan::Context &ctx) override;

	void serialize(const Swan::Context &ctx, Proto::Builder w);
	void deserialize(const Swan::Context &ctx, Proto::Reader r);

private:
	struct MovingItem {
		Swan::Item *item;
		Swan::Direction from;
		Swan::Direction to;
		int timer = 0;
	};

	struct InboxItem {
		Swan::Item *item;
		Swan::Direction from;
	};

	class Inbox final: public Swan::InventoryTrait::Inventory {
	public:
		Swan::ItemStack take(int slot) override
		{
			return {};
		}

		Swan::ItemStack set(int slot, Swan::ItemStack stack) override
		{
			return stack;
		}

		Swan::ItemStack insertInto(Swan::ItemStack stack, int from, int to) override;
		Swan::ItemStack insertSided(
			Swan::Direction dir, Swan::ItemStack stack) override;

		std::span<const Swan::ItemStack> content() const override
		{
			return {};
		}

		std::optional<InboxItem> content_;
	};

	void moveItemOut(const Swan::Context &ctx, size_t index);

	TileEntity tileEntity_;
	Inbox inbox_;
	std::vector<MovingItem> content_;
};

}
