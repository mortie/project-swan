#pragma once

#include "MPServer.h"
#include <vector>

namespace Swan {

class World;

class GameServer {
public:
	GameServer(World *world, std::vector<std::string> modIDs):
		world_(world),
		modIDs_(std::move(modIDs))
	{}
	~GameServer() { server_.end(); }

	void tick(float dt);

	void listen(const char *host, int port) { server_.listen(host, port); }
	void end(const char *reason) { server_.end(reason); }
	bool running() { return server_.running(); }

private:
	void onMessageFromClient(
		const MPServer::ClientInfo &client,
		mp_proto::ClientToServer::Reader &r);

	MPServer server_;
	World *world_;
	std::vector<std::string> modIDs_;
};

}
