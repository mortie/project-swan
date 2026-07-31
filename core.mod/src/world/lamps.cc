#include "lamps.h"
#include "tileentities/IncandescentLampTileEntity.h"
#include "world/util.h"

namespace CoreMod {

void registerIncandescentLamp(Swan::Mod &mod)
{
	mod.registerEntity<IncandescentLampTileEntity>("tile::incandescent-lamp");

	mod.registerTile({
		.name = "incandescent-lamp",
		.image = "core::tiles/incandescent-lamp",
		.isSolid = false,
		.breakableBy = Swan::Tool::HAND,
		.breakSound = "core::break/glass",
		.droppedItem = "core::incandescent-lamp",
		.tileEntity = "core::tile::incandescent-lamp",
		.onSpawn = denyIfFloating,
		.onTileUpdate = breakIfFloating,
	});
}

}
