#include "ChestTileEntity.h"
#include "world/util.h"

namespace CoreMod {

void ChestTileEntity::serialize(Swan::Ctx &ctx, Proto::Builder w)
{
	// Close the chest on load if it's open
	auto &tile = ctx.plane.tiles().get(tileEntity_.pos);
	if (tile.name.str().ends_with("::open")) {
		auto newName = tile.name.str().substr(0, tile.name.size() - 6);
		ctx.plane.tiles().set(tileEntity_.pos, newName);
	}

	tileEntity_.serialize(w.initTileEntity());
	inventory_.serialize(w.initInventory());
}

void ChestTileEntity::deserialize(Swan::Ctx &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());
	inventory_.deserialize(ctx, r.getInventory());
}

void ChestTileEntity::onDespawn(Swan::Ctx &ctx)
{
	Swan::info << "Despawning chest at " << tileEntity_.pos;
	for (auto stack: inventory_.content_) {
		if (stack.empty()) {
			continue;
		}

		for (int i = 0; i < stack.count(); ++i ){
			dropItem(ctx, tileEntity_.pos, *stack.item());
		}
	}
}

}
