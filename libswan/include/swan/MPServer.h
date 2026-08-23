#pragma once

#include <memory>
#include <string>
#include <cstdint>
#include <swan/util.h>
#include <swan/log.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <kj/array.h>
#include <kj/io.h>

#include "multiplayer.capnp.h"

namespace Swan {

class MPServer {
public:
	MPServer();
	~MPServer();

	struct ClientInfo {
		std::string identifier;
		std::string nick;
		bool requestWorld = false;
	};

	bool listen(const char *host, int port);
	void end(const char *reason = "Server shutting down");
	bool running() { return impl_.get(); }
	void tick(float dt);

	const ClientInfo *receive(mp_proto::ClientToServer::Reader &r);

	operator bool() const { return impl_.get(); }

private:
	class Impl;
	struct Client;

	std::unique_ptr<Impl> impl_;
};

}
