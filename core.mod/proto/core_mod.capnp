@0xd255dab065b919c3;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("CoreMod::proto");

using import "/swan.capnp".PhysicsBody;
using import "/swan.capnp".Inventory;

struct DynamiteEntity {
	body @0 :PhysicsBody;
	fuse @1 :Float32;
}

struct FallingTileEntity {
	body @0 :PhysicsBody;
	tileID @1 :UInt32;
}

struct ItemStackEntity {
	body @0 :PhysicsBody;
	lifetime @1 :Float32;
	item @2 :Text;
}

struct PlayerEntity {
	body @0 :PhysicsBody;
	inventory @1 :Inventory;
}

struct SpiderEntity {
	body @0 :PhysicsBody;
}
