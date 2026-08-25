#include "MPServer.h"

#include <cassert>
#include <chrono>
#include <swan/util.h>
#include <swan/log.h>
#include <SDL3_net/SDL_net.h>

#include "MPSocket.h"
#include "SDL3/SDL_error.h"
#include "capnp/message.h"
#include "multiplayer.capnp.h"

namespace Swan {

using Server = CPtr<NET_Server, NET_DestroyServer>;
using Address = CPtr<NET_Address, NET_UnrefAddress>;

struct MPServer::Client {
	enum State {
		HANDSHAKING,
		IDLE,
		PINGED,
		KICKED,
	};

	State state = HANDSHAKING;
	float timer = 5;
	ClientInfo info;
	MPSocket sock;
};

class MPServer::Impl {
public:
	Impl()
	{
		// Need to zero the scratch space.
		// There doesn't seem to be a nice "allocate a zeroed kj array" function.
		memset(&scratch_.front(), 0, scratch_.asBytes().size());
	}

	bool listen(const char *host, int port);

	void tick(float dt);
	void end(const char *reason);

	mp_proto::ServerToClient::Builder builder()
	{
		using MMB = capnp::MallocMessageBuilder;
		builder_.~MMB();
		new (&builder_) MMB(scratch_);
		return builder_.initRoot<mp_proto::ServerToClient>();
	}

	const ClientInfo *receive(mp_proto::ClientToServer::Reader &r);
	void kick(Client &client, const char *reason);

private:
	Server server_;
	std::vector<std::unique_ptr<Client>> clients_;

	// Scratch buffers for encoding and sending messages
	kj::Array<capnp::word> scratch_ = kj::heapArray<capnp::word>(1024);
	kj::VectorOutputStream stream_;
	capnp::MallocMessageBuilder builder_;

	size_t receiveIndex_ = 0;
	uint64_t nextClientID_ = 1;

	friend MPServer;
};

bool MPServer::Impl::listen(const char *host, int port)
{
	Address addr;
	if (host) {
		addr.reset(NET_ResolveHostname(host));
		if (!addr) {
			warn << "Failed to resolve host '" << host << "': " << SDL_GetError();
			return false;
		}

		NET_Status status = NET_WaitUntilResolved(addr.get(), 5000);
		if (status != NET_SUCCESS) {
			warn << "Failed to resolve host '" << host << "'";
			return false;
		}

		info << "Creating MPServer listening on " << host << ':' << port << "...";
	} else {
		info << "Creating MPServer listening on all interfaces, port " << port << "...";
	}

	server_.reset(NET_CreateServer(addr.get(), Uint16(port), 0));
	if (!server_) {
		warn << "Failed to create server: " << SDL_GetError();
		return false;
	}

	return true;
}

void MPServer::Impl::end(const char *reason)
{
	auto root = builder();
	root.initShutdown().setReason(reason);
	MPSocket::encode(builder_, stream_);

	// Send shutdown messages to all clients
	for (auto &client: clients_) {
		client->sock.send(stream_);
	}

	// Wait a bit for data to be sent
	auto start = std::chrono::steady_clock::now();
	for (auto &client: clients_) {
		client->sock.drain(1000);
		if (std::chrono::steady_clock::now() - start > std::chrono::seconds(2)) {
			break;
		}
	}

	clients_.clear();
}

void MPServer::Impl::tick(float dt)
{
	receiveIndex_ = 0;

	// Accept new clients
	NET_StreamSocket *newSock = nullptr;
	if (!NET_AcceptClient(server_.get(), &newSock)) {
		warn << "Failed to accept clients: " << SDL_GetError();
		return;
	}

	// Add new clients
	if (newSock) {
		Address addr(NET_GetStreamSocketAddress(newSock));
		const char *addrStr = NET_GetAddressString(addr.get());
		if (!addrStr) {
			warn << "Client connected, but failed to get address: " << SDL_GetError();
		} else {
			info << "Client connected with address: " << addrStr;
		}
		auto client = std::make_unique<Client>();
		client->sock.reset(newSock);
		clients_.push_back(std::move(client));
	}

	// Handle client timing stuff
	for (size_t i = 0; i < clients_.size();) {
		auto &client = *clients_[i];
		client.timer -= dt;
		if (client.timer > 0) {
			i += 1;
			continue;
		}

		if (client.state == Client::KICKED) {
			clients_[i] = std::move(clients_.back());
			clients_.pop_back();
			continue;
		}

		auto root = builder();
		switch (client.state) {
		case Client::HANDSHAKING:
			warn << "Client timed out during handshake";
			root.initKick().setReason("Timed out waiting for handshake");
			client.state = Client::KICKED;
			client.timer = 2;
			break;
		case Client::IDLE:
			root.setPing();
			client.state = Client::PINGED;
			client.timer = 5;
			break;
		case Client::PINGED:
			warn << "Client timed out";
			root.initKick().setReason("Timed out");
			client.state = Client::KICKED;
			client.timer = 2;
			break;
		case Client::KICKED:
			// Handled earlier
			break;
		}

		client.sock.encodeAndSend(builder_, stream_);
		i += 1;
	}
}

const MPServer::ClientInfo *
MPServer::Impl::receive(mp_proto::ClientToServer::Reader &r)
{
	while (receiveIndex_ < clients_.size()) {
		auto &client = *clients_[receiveIndex_];
		if (!client.sock.receive(r)) {
			receiveIndex_ += 1;
			continue;
		}

		if (client.state == Client::HANDSHAKING) {
			if (r.isQuit()) {
				info << "Client quit during handshake";
				client.state = Client::KICKED;
				client.timer = 0;
				continue;
			}

			if (!r.isHello()) {
				warn << "Client in handshaking state sent non-hello message";
				kick(client, "Protocol sequence error");
				continue;
			}

			auto clientHello = r.getHello();
			auto identifier = clientHello.getIdentifier();
			auto nick = clientHello.getNick();
			if (identifier == "" || nick == "") {
				warn << "Client hello missing identifier and nick";
				kick(client, "Protocol hello error");
				continue;
			}

			Address addr(NET_GetStreamSocketAddress(client.sock.get()));
			const char *addrStr = NET_GetAddressString(addr.get());

			(info
				<< "Client handshake completed: Client " << nextClientID_
				<< " from IP "
				<< addrStr << " connected with: "
				<< "nick '" << nick.cStr()
				<< "', identifier '" << identifier.cStr()
				<< "', using host '" << clientHello.getHost().cStr() << "'");
			client.info.id.id = nextClientID_++;
			client.info.identifier = identifier.cStr();
			client.info.nick = nick.cStr();
			client.info.requestWorld = clientHello.getRequestWorld();
			client.state = Client::IDLE;
			client.timer = 5;

			// Respond to let the client know it's good,
			// and to let it know which mods to load
			auto root = builder();
			root.initHello();
			client.sock.encodeAndSend(builder_, stream_);

			return &client.info;
		}

		// Don't tell the caller about pong messages
		if (r.isPong()) {
			client.state = Client::IDLE;
			client.timer = 5;
			continue;
		}

		// When the client quit,
		// treat it as if we kicked the client.
		// The next tick will clean it out.
		if (r.isQuit()) {
			info << "Client " << client.info.identifier << " quit";
			client.state = Client::KICKED;
			client.timer = 0;
			receiveIndex_ += 1;
			continue;
		}
	}

	return nullptr;
}

void MPServer::Impl::kick(Client &client, const char *reason)
{
	client.state = Client::KICKED;
	client.timer = 2;
	auto root = builder();
	root.initKick().setReason(reason);
	client.sock.encodeAndSend(builder_, stream_);
}

MPServer::MPServer() = default;
MPServer::~MPServer() = default;

bool MPServer::listen(const char *host, int port)
{
	end("Server restarting");
	impl_ = std::make_unique<Impl>();
	if (!impl_->listen(host, port)) {
		impl_.reset();
		return false;
	}

	return true;
}

void MPServer::end(const char *reason)
{
	if (impl_) {
		impl_->end(reason);
		impl_.reset();
	}
}

void MPServer::tick(float dt)
{
	impl_->tick(dt);
}

const MPServer::ClientInfo *
MPServer::receive(mp_proto::ClientToServer::Reader &r)
{
	return impl_->receive(r);
}

mp_proto::ServerToClient::Builder MPServer::builder()
{
	return impl_->builder();
}

void MPServer::send(
	ClientID id,
	const mp_proto::ServerToClient::Builder &)
{
	for (auto &c: impl_->clients_) {
		if (c->info.id.id != id.id) {
			continue;
		}

		c->sock.encodeAndSend(impl_->builder_, impl_->stream_);
		break;
	}
}

void MPServer::broadcast(const mp_proto::ServerToClient::Builder &)
{
	MPSocket::encode(impl_->builder_, impl_->stream_);

	for (auto &c: impl_->clients_) {
		if (c->info.id.id == 0) {
			continue;
		}

		c->sock.send(impl_->stream_);
	}
}

}
