#pragma once

#include <memory>
#include <string>

#include "multiplayer.capnp.h"

namespace Swan {

class MPClient {
public:
	struct Options {
		std::string host;
		int port;
		std::string nick;
		std::string identifier;
		bool requestWorld = false;
	};

	enum StateTag {
		CONNECTING,
		HANDSHAKING,
		CONNECTED,
		CLOSED,
		KICKED,
		SHUTDOWN,
		ERROR,
	};

	struct State {
		StateTag tag;
		std::string reason = "";
	};

	MPClient();
	~MPClient();

	void connect(Options opts);
	void end();
	void tick(float dt);
	bool receive(mp_proto::ServerToClient::Reader &r);

	State state();

private:
	class Impl;

	std::unique_ptr<Impl> impl_;
};

}
