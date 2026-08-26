#pragma once

#include "MPServer.h"
#include "Tile.h"
#include "WorldPlane.h"
#include "common.h"
#include <vector>

namespace Swan {

class World;

class GameServer {
public:
	struct ConnectedClient {
		MPServer::ClientInfo info;
		WorldPlane::ID plane;
	};

	GameServer(World *world, std::vector<std::string> modIDs):
		world_(world),
		modIDs_(std::move(modIDs))
	{}
	~GameServer() { server_.end(); }

	void onTileChange(WorldPlane::ID plane, TilePos pos, Tile::ID newID);

	void tick(float dt);

	void listen(const char *host, int port) { server_.listen(host, port); }
	void end(const char *reason) { server_.end(reason); }
	bool running() { return server_.running(); }

private:
	void onMessageFromClient(
		const MPServer::ClientInfo &client,
		mp_proto::ClientToServer::Reader &r);
	void broadcastTickUpdateForPlane(WorldPlane::ID plane);

	void broadcastToPlane(
		WorldPlane::ID plane,
		const mp_proto::ServerToClient::Builder &root);

	MPServer server_;
	World *world_;
	std::vector<std::string> modIDs_;
	std::vector<ConnectedClient> clients_;
};

}
