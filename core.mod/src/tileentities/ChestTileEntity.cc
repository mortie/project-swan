#include "ChestTileEntity.h"
#include "world/util.h"

namespace CoreMod {

void ChestTileEntity::serialize(const Swan::Context &ctx, Proto::Builder w)
{
	tileEntity_.serialize(w.initTileEntity());
	inventory_.serialize(w.initInventory());
}

void ChestTileEntity::deserialize(const Swan::Context &ctx, Proto::Reader r)
{
	tileEntity_.deserialize(r.getTileEntity());
	inventory_.deserialize(ctx, r.getInventory());
}

void ChestTileEntity::onDespawn(const Swan::Context &ctx)
{
	Swan::info << "Despawning chest at " << tileEntity_.pos;
	for (auto stack: inventory_.content) {
		if (stack.empty()) {
			continue;
		}

		for (int i = 0; i < stack.count(); ++i ){
			dropItem(ctx, tileEntity_.pos, *stack.item());
		}
	}
}

}
