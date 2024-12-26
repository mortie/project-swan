@0xd255dab065b919c3;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("CoreMod::proto");

using import "/swan.capnp".BasicPhysicsBody;
using import "/swan.capnp".BasicInventory;
using import "/swan.capnp".TileEntity;
using import "/swan.capnp".ItemStack;
using import "/swan.capnp".Direction;

struct DynamiteEntity {
	body @0 :BasicPhysicsBody;
	fuse @1 :Float32;
}

struct FallingTileEntity {
	body @0 :BasicPhysicsBody;
	tile @1 :Text;
}

struct ItemStackEntity {
	body @0 :BasicPhysicsBody;
	lifetime @1 :Float32;
	item @2 :Text;
}

struct PlayerEntity {
	body @0 :BasicPhysicsBody;
	inventory @1 :BasicInventory;
	heldStack @2 :ItemStack;
}

struct SpiderEntity {
	body @0 :BasicPhysicsBody;
}

struct ItemFanTileEntity {
	tileEntity @0 :TileEntity;
	direction @1 :Direction;
}

struct ItemPipeTileEntity {
	tileEntity @0 :TileEntity;
	inbox @1 :InboxItem;
	contents @2 :List(MovingItem);

	struct InboxItem {
		from @0 :Direction;
		item @1 :Text;
	}

	struct MovingItem {
		item @0 :Text;
		from @1 :Direction;
		to @2 :Direction;
		timer @3 :UInt8;
	}
}
