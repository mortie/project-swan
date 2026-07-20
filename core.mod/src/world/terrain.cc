#include "terrain.h"
#include "tiles.h"
#include "world/util.h"

namespace CoreMod {

void registerTerrain(Swan::Mod &mod)
{
	auto slabFluidCollision = std::make_shared<Swan::FluidCollision>(0b1111'1111'0000'0000);

	registerBackgroundConnected47(mod, {
		.name = "background",
		.image = "core::tiles/background",
		.isSolid = false,
		.isOpaque = false,
	});

	mod.registerTile({
		.name = "stone",
		.image = "core::tiles/stone",
		.stepSound = "core::step/stone",
		.droppedItem = "core::stone",
	});

	mod.registerTile({
		.name = "dirt",
		.image = "core::tiles/dirt",
		.stepSound = "core::step/grass",
		.placeSound = "core::place/dirt",
		.droppedItem = "core::dirt",
	});

	mod.registerTile({
		.name = "dirt-slab",
		.image = "core::tiles/dirt-slab",
		.isSolid = false,
		.isSupportV = false,
		.isSupportH = false,
		.stepSound = "core::step/grass",
		.placeSound = "core::place/dirt",
		.droppedItem = "core::dirt",
		.fluidCollision = slabFluidCollision,
	});

	mod.registerTile({
		.name = "grass",
		.image = "core::tiles/grass",
		.stepSound = "core::step/grass",
		.placeSound = "core::place/dirt",
		.droppedItem = "core::dirt",
		.onTileUpdate = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
			if (ctx.plane.tiles().get(pos.add(0, -1)).isOpaque()) {
				ctx.plane.tiles().setID(pos, tiles::dirt);
			}
		},
		.onWorldTick = +[](Swan::Ctx &ctx, Swan::TilePos pos) {
			// Convert self into dirt if there's fluid
			auto fluid = ctx.plane.fluids().getAtPos(pos.as<float>().add(0.5, -0.2));
			if (fluid.density > 0) {
				ctx.plane.tiles().setID(pos, tiles::dirt);
				return;
			}

			auto neighbours = {
				pos.add(-1, 0),
				pos.add(1, 0),
				pos.add(-1, -1),
				pos.add(1, -1),
				pos.add(-1, 1),
				pos.add(1, 1),
			};

			// Convert nearby dirt into grass
			for (auto pos: neighbours) {
				if (ctx.plane.tiles().getID(pos) != tiles::dirt) {
					continue;
				}

				auto &above = ctx.plane.tiles().get(pos.add(0, -1));
				if (
					above.isSolid() ||
					above.id == tiles::grassSlab ||
					above.id == tiles::dirtSlab)
				{
					continue;
				}

				ctx.plane.tiles().setID(pos, tiles::grass);
				return;
			}
		},
	});

	mod.registerTile({
		.name = "grass-slab",
		.image = "core::tiles/grass-slab",
		.isSolid = false,
		.isSupportV = false,
		.isSupportH = false,
		.stepSound = "core::step/grass",
		.placeSound = "core::place/dirt",
		.droppedItem = "core::dirt",
		.fluidCollision = slabFluidCollision,
	});

	registerShovelable(mod, {
		.name = "snow",
		.image = "core::tiles/snow",
		.stepSound = "core::step/sand",
		.placeSound = "core::step/sand2",
		.breakSound = "core::step/sand1",
		.droppedItem = "core::snowball",
		.onTileUpdate = fallIfFloating,
	});

	registerShovelable(mod, {
		.name = "sand",
		.image = "core::tiles/sand",
		.stepSound = "core::step/sand",
		.placeSound = "core::step/sand2",
		.breakSound = "core::step/sand2",
		.droppedItem = "core::sand-pile",
		.onTileUpdate = fallIfFloating,
	});

}

}
