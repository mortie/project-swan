#pragma once

#include "EntityCollection.h"
#include "MPServer.h"
#include "Tile.h"
#include "WorldPlane.h"
#include "common.h"
#include "kjutil.h"

#include <kj/array.h>
#include <kj/io.h>
#include <kj/vector.h>
#include <capnp/common.h>
#include <vector>

namespace Swan {

class World;
class Game;

class GameServer {
public:
	struct ConnectedClient {
		MPServer::ClientInfo info;
		WorldPlane::ID plane;
		EntityRef ref;
	};

	GameServer(World *world, Game *game, std::vector<std::string> modIDs):
		world_(world),
		game_(game),
		modIDs_(std::move(modIDs))
	{}
	~GameServer() { server_.end(); }

	void onTileChange(
		WorldPlane::ID plane,
		TilePos pos,
		Tile::ID newID);
	void onBackgroundTileChange(
		WorldPlane::ID plane,
		TilePos pos,
		Tile::ID newID);

	void tick(float dt);

	void listen(const char *host, int port) { server_.listen(host, port); }
	void end(const char *reason) { server_.end(reason); }
	bool running() { return server_.running(); }

private:
	struct TileChange {
		TilePos pos;
		Tile::ID newID;
	};

	struct FluidChange {
		FluidPos pos;
		Fluid::ID newID;
	};

	struct PlaneChanges {
		std::vector<TileChange> tileChanges;
		std::vector<TileChange> backgroundChanges;
	};

	PlaneChanges &planeChanges(WorldPlane::ID plane) {
		if (plane >= planeChanges_.size()) {
			planeChanges_.resize(plane + 1);
		}

		return planeChanges_[plane];
	}

	void onMessageFromClient(
		const MPServer::ClientInfo &client,
		mp_proto::ClientToServer::Reader &r);
	void broadcastTickUpdateForPlane(WorldPlane &plane);

	void broadcastToPlane(
		WorldPlane::ID plane,
		const mp_proto::ServerToClient::Builder &root);

	MPServer server_;
	World *world_;
	Game *game_;
	std::vector<std::string> modIDs_;
	std::vector<ConnectedClient> clients_;
	std::vector<PlaneChanges> planeChanges_;

	kj::VectorOutputStream stream_;
	kj::Array<capnp::word> scratch_ = kjZeroedArray<capnp::word>(1024);
};

}
