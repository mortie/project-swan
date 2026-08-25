@0x8923d61ec8121042;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("Swan::mp_proto");

using import "./swan.capnp".WorldPlane;
using import "./swan.capnp".EntitySystem;

struct ClientToServer {
	union {
		hello @0 :Hello;
		quit @1 :Void;
		pong @2 :Void;
	}

	struct Hello {
		nick @0 :Text;

		# This works like the HTTP Host header:
		# it should be the host string
		# which was used to resolve the IP address.
		host @1 :Text;

		# This is a unique ID which the client should be identified by.
		# Used for tracking client data across reconnects.
		identifier @2 :Text;

		requestWorld @3 :Bool;
	}
}

struct ServerToClient {
	union {
		hello @0 :Hello;
		shutdown @1 :Shutdown;
		kick @2 :Kick;
		ping @3 :Void;
		worldSync @4 :WorldSync;
		tick @5 :Tick;
	}

	struct Hello {}

	struct Shutdown {
		reason @0 :Text;
	}

	struct Kick {
		reason @0 :Text;
	}

	struct WorldSync {
		modIDs @0 :List(Text);
		tiles @1 :List(Text);
		currentPlaneIndex @2 :UInt32;
		currentPlane @3 :WorldPlane;
		worldSeed @4 :UInt32;
	}

	struct Tick {
		updatedEntityCollections @0 :List(EntityCollectionUpdate);
	}
}

struct EntityCollectionUpdate {
	index @0 :UInt32;
	updatedEntities @1 :List(EntitySystem.Entity);
}
