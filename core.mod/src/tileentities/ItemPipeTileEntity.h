#pragma once

#include <swan/swan.h>

namespace CoreMod {

class ItemPipeTileEntity final: public Swan::Entity,
	public Swan::TileEntityTrait,
	public Swan::InventoryTrait {
public:
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

	void onDespawn(const Swan::Context &ctx) override;

	void serialize(const Swan::Context &ctx, sbon::ObjectWriter w) override;
	void deserialize(const Swan::Context &ctx, sbon::ObjectReader r) override;

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

	class Inbox: public Swan::InventoryTrait::Inventory {
	public:
		int size() override
		{
			return 0;
		}

		Swan::ItemStack get(int slot) override
		{
			return {};
		}

		Swan::ItemStack set(int slot, Swan::ItemStack stack) override
		{
			return stack;
		}

		Swan::ItemStack insert(Swan::ItemStack stack) override;
		Swan::ItemStack insert(
			Swan::Direction dir, Swan::ItemStack stack) override;

		std::optional<InboxItem> contents_;
	};

	void moveItemOut(const Swan::Context &ctx, size_t index);

	TileEntity tileEntity_;
	Inbox inbox_;
	std::vector<MovingItem> contents_;
};

}
