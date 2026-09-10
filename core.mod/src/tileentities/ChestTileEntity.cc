#include "ChestTileEntity.h"
#include "core_mod.capnp.h"
#include "world/util.h"

namespace CoreMod {

void ChestTileEntity::serialize(Swan::Ctx &ctx, capnp::MessageBuilder &mb)
{
	auto w = mb.initRoot<proto::ChestTileEntity>();
	// Close the chest on load if it's open
	auto &tile = ctx.plane.tiles().get(tileEntity_.pos);
	if (tile.name.str().ends_with("::open")) {
		auto newName = tile.name.str().substr(0, tile.name.size() - 6);
		ctx.plane.tiles().set(tileEntity_.pos, newName);
	}

	tileEntity_.serialize(w.initTileEntity());
	inventory_.serialize(w.initInventory());
}

void ChestTileEntity::deserialize(Swan::Ctx &ctx, capnp::MessageReader &mr)
{
	auto r = mr.getRoot<proto::ChestTileEntity>();
	tileEntity_.deserialize(r.getTileEntity());
	inventory_.deserialize(ctx, r.getInventory());
}

void ChestTileEntity::onDespawn(Swan::Ctx &ctx)
{
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
