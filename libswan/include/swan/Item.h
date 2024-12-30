#pragma once

#include <string>

#include "Tile.h"
#include "traits/InventoryTrait.h"
#include "Tool.h"

namespace Swan {

struct Item {
	struct Traits {
		virtual ~Traits() = default;
	};

	struct Builder {
		std::string name;
		std::string image = "@::invalid";
		int maxStack = 64;
		std::optional<std::string> tile = std::nullopt;
		ToolSet tool = Tool::NONE;

		void (*onActivate)(
			const Context &ctx, InventorySlot slot, Vec2 pos, Vec2 dir) = nullptr;
	};

	Tile::ID id;
	std::string name;
	int maxStack;
	Tile *tile;
	ToolSet tool;

	bool hidden = true;

	void (*onActivate)(
		const Context &ctx, InventorySlot slot, Vec2 pos, Vec2 dir);

	Item() = default;
	Item(Tile::ID id, std::string name, const Builder &builder):
		id(id), name(name), maxStack(builder.maxStack), tile(nullptr),
		tool(builder.tool),
		onActivate(builder.onActivate)
	{}
};

}
