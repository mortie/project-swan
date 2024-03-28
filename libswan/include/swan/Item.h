#pragma once

#include <string>

#include "Tile.h"
#include "traits/InventoryTrait.h"

namespace Swan {

struct Item {
public:
	struct Traits {
		virtual ~Traits() = default;
	};

	struct Builder {
		std::string name;
		std::string image;
		int maxStack = 64;
		std::optional<std::string> tile;

		void (*onActivate)(
			const Context &ctx, InventorySlot slot, Vec2 pos, Vec2 dir);
	};

	const Tile::ID id;
	const std::string name;
	const int maxStack;
	const Tile *tile;

	void (*const onActivate)(
		const Context &ctx, InventorySlot slot, Vec2 pos, Vec2 dir);


	Item(Tile::ID id, std::string name, const Builder &builder):
		id(id), name(name), maxStack(builder.maxStack), tile(nullptr),
		onActivate(builder.onActivate)
	{}
};

}
