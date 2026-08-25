@0x8923d61ec8121042;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace("Swan::mp_proto");

using import "./swan.capnp".WorldPlane;

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
	}

	struct Hello {
		# List of mod IDs (name@version) which the server uses
		mods @0 :List(Text);
	}

	struct Shutdown {
		reason @0 :Text;
	}

	struct Kick {
		reason @0 :Text;
	}

	struct WorldSync {
		tiles @0 :List(Text);
		currentPlaneIndex @1 :UInt32;
		currentPlane @2 :WorldPlane;
	}
}
