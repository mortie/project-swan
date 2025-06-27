#include <swan/swan.h>

#include "world/util.h"
#include "entities/ItemStackEntity.h"

namespace CoreMod {

template<int N>
void activateClay(Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta)
{
	if constexpr (N == 0) {
		ctx.plane.tiles().setID(pos, Swan::World::AIR_TILE_ID);
	} else{
		std::string next = Swan::cat("core::clay-tile::", std::to_string(N - 1));
		ctx.plane.tiles().set(pos, next);
	}

	auto item = ctx.plane.entities().spawn<ItemStackEntity>(
		pos.as<float>().add(0.5, -0.3), &ctx.world.getItem("core::clay"));
	item.trait<Swan::PhysicsBodyTrait>()->addVelocity({0, -3.0f});
}

template<int N, bool Default = true>
void registerClayTile(Swan::Mod &mod)
{
	std::string name;
	if (Default) {
		name = "clay-tile";
	} else {
		name = Swan::cat("clay-tile::", std::to_string(N));
	}

	mod.registerTile({
		.name = std::move(name),
		.image = "core::tiles/clay",
		.stepSound = "core::sounds/step/sand",
		.droppedItem = "core::clay",
		.onTileUpdate = fallIfFloating,
		.onActivate = activateClay<N>,
	});

	if constexpr (N > 0) {
		registerClayTile<N - 1, false>(mod);
	}
}

void registerClay(Swan::Mod &mod)
{
	registerClayTile<4>(mod);
}

}
