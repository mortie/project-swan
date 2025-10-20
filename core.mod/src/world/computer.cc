#include "computer.h"
#include "tileentities/ComputerTileEntity.h"

namespace CoreMod {

void registerComputer(Swan::Mod &mod)
{
	mod.registerEntity<ComputerTileEntity>("tile::computer");

	mod.registerTile({
		.name = "computer",
		.image = "core::tiles/computer",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.droppedItem = "core::computer",
		.tileEntity = "core::tile::computer",
		.onActivate = +[](Swan::Ctx &ctx, Swan::TilePos pos, Swan::Tile::ActivateMeta) {
			auto ref = ctx.plane.entities().getTileEntity(pos);
			auto *computer = ref.as<ComputerTileEntity>();
			if (computer) {
				computer->activate();
			}
		},
	});
}

}
