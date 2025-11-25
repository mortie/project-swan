@0x8e283591e1813af2;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("Swan::proto");

struct Vec2 {
	x @0 :Float32;
	y @1 :Float32;
}

struct Vec2i {
	x @0 :Int32;
	y @1 :Int32;
}

struct Color {
	r @0 :UInt8;
	g @1 :UInt8;
	b @2 :UInt8;
	a @3 :UInt8;
}

struct Direction {
	value @0 :UInt8;
}

struct DirectionSet {
	value @0 :UInt8;
}

struct EntityRef {
	collection @0 :Text;
	id @1 :UInt32;
}

struct ItemStack {
	item @0 :Text;
	count @1 :UInt8;
}

struct BasicPhysicsBody {
	pos @0 :Vec2;
	vel @1 :Vec2;
}

struct BasicInventory {
	size @0 :UInt32;
	slots @1 :List(Slot);

	struct Slot {
		index @0 :UInt32;
		stack @1 :ItemStack;
	}
}

struct TileEntity {
	pos @0 :Vec2i;
}

struct World {
	tiles @0 :List(Text);
	planes @1 :List(WorldPlane);
	player @2 :EntityRef;
	currentPlane @3 :UInt32;
	seed @4 :UInt32;
}

struct WorldPlane {
	worldGen @0 :Text;
	chunks @1 :List(Chunk);
	entitySystem @2 :EntitySystem;
	fluidSystem @3 :FluidSystem;
}

struct Chunk {
	pos @0 :Vec2i;
	compression @1 :Compression;
	data @2 :Data;

	enum Compression {
		none @0;
		gzip @1;
	}
}

struct EntitySystem {
	collections @0 :List(Collection);
	tileEntities @1 :List(TileEntity);

	struct Collection {
		name @0 :Text;
		nextID @1 :UInt64;
		entities @2 :List(Entity);
	}

	struct Entity {
		id @0 :UInt64;
		data @1 :Data;
	}

	struct TileEntity {
		pos @0 :Vec2i;
		ref @1 :EntityRef;
	}
}

struct FluidSystem {
	updates @0 :List(Vec2i);
	particles @1 :List(Particle);

	struct Particle {
		pos @0 :Vec2;
		vel @1 :Vec2;
		id @2 :UInt8;
		remainingTime @3 :UInt8;
	}
}
