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

	struct ActivateMeta {
		EntityRef activator;
		ItemStack &stack;
		Vec2 direction;
	};

	struct Builder {
		std::string name;
		std::string image = "@::invalid";
		int maxStack = 64;
		std::optional<std::string> tile = std::nullopt;
		ToolSet tool = Tool::NONE;
		float lightLevel = 0;

		void (*onActivate)(Ctx &ctx, ActivateMeta meta) = nullptr;
	};

	Tile::ID id;
	std::string name;
	std::string displayName;
	int maxStack;
	Tile *tile;
	ToolSet tool;
	float lightLevel;

	bool hidden = true;
	float yOffset = 0;

	void (*onActivate)(Ctx &ctx, ActivateMeta meta) = nullptr;

	Item() = default;
	Item(Tile::ID id, std::string name, const Builder &builder):
		id(id), name(name), maxStack(builder.maxStack), tile(nullptr),
		tool(builder.tool), lightLevel(builder.lightLevel),
		onActivate(builder.onActivate)
	{}
};

}
