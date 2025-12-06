@0xd255dab065b919c3;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("CoreMod::proto");

using import "/swan.capnp".BasicPhysicsBody;
using import "/swan.capnp".BasicInventory;
using import "/swan.capnp".TileEntity;
using import "/swan.capnp".ItemStack;
using import "/swan.capnp".Direction;
using import "/swan.capnp".EntityRef;
using import "/swan.capnp".Vec2i;

struct CraftingInventory {
	discoveredRecipes @0 :List(Text);
}

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
	spawnPoint @3 :Vec2i;
	inventorySlot @4 :UInt8;
	health @5 :UInt8;
	craftingInventory @6 :CraftingInventory;
}

struct SpiderEntity {
	body @0 :BasicPhysicsBody;
}

struct BonfireTileEntity {
	tileEntity @0 :TileEntity;
	ongoing @1 :List(OngoingBurn);

	struct OngoingBurn {
		inputs @0 :List(EntityRef);
		output @1 :ItemStack;
		timer @2 :Float32;
	}
}

struct CrucibleTileEntity {
	tileEntity @0 :TileEntity;
	drawSupports @1 :Bool;
	temperature @2 :Float32;
	items @3 :List(Text);
	progress @4 :Progress;
	targetTemperature @5 :Float32;

	struct Progress {
		timer @0 :Float32;
		output @1 :ItemStack;
	}
}

struct ChestTileEntity {
	tileEntity @0 :TileEntity;
	inventory @1 :BasicInventory;
}

struct ComputerTileEntity {
	tileEntity @0 :TileEntity;
	assembly @1 :Text;
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

struct AqueductTileEntity {
	tileEntity @0 :TileEntity;
	fluidLevel @1 :Float32;
	fluidType @2 :Text;
}
