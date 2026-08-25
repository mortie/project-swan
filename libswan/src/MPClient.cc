#include "MPClient.h"

#include <swan/util.h>
#include <swan/log.h>
#include <SDL3_net/SDL_net.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_timer.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <kj/array.h>
#include <kj/io.h>

#include "multiplayer.capnp.h"
#include "MPSocket.h"

namespace Swan {

using Address = CPtr<NET_Address, NET_UnrefAddress>;

class MPClient::Impl {
public:
	Impl(Options opts):
		opts_(std::move(opts))
	{
		// Need to zero the scratch space.
		// There doesn't seem to be a nice "allocate a zeroed kj array" function.
		memset(&scratch_.front(), 0, scratch_.asBytes().size());
	}

	void tick(float dt);
	void end();
	State state() { return state_; }

	capnp::MallocMessageBuilder builder()
	{ return capnp::MallocMessageBuilder(scratch_); }

	bool send(capnp::MessageBuilder &mb);
	bool receive(mp_proto::ServerToClient::Reader &r);

private:
	void tickInitiateConnection();
	void tickHandshake();

	void error(State newState = {ERROR});
	void error(const char *reason)
	{
		error({ERROR, reason});
	}

	Options opts_;
	State state_ = {CONNECTING};

	float timer_ = 0;
	Address addr_;
	MPSocket sock_;

	// Scratch buffers for encoding and sending messages
	kj::Array<capnp::word> scratch_ = kj::heapArray<capnp::word>(1024);
	kj::VectorOutputStream stream_;
};

void MPClient::Impl::tick(float dt)
{
	switch (state_.tag) {
	case CONNECTING:
		tickInitiateConnection();
		break;
	case HANDSHAKING:
		tickHandshake();
		break;
	case CONNECTED:
	case CLOSED:
	case KICKED:
		break;
	case ERROR:
	case SHUTDOWN:
		timer_ -= dt;
		if (timer_ < 0) {
			state_ = {CONNECTING};
		}
		break;
	}
}

void MPClient::Impl::end()
{
	// Tell the server that we're intentionally going away,
	// so that it doesn't look like an error
	if (state_.tag == CONNECTED || state_.tag == HANDSHAKING) {
		auto mb = builder();
		auto root = mb.initRoot<mp_proto::ClientToServer>();
		root.setQuit();
		sock_.encodeAndSend(mb, stream_);
		sock_.drain(1000);
	}
}

bool MPClient::Impl::send(capnp::MessageBuilder &mb)
{
	if (state_.tag != CONNECTED) {
		return false;
	}

	if (!sock_.encode(mb, stream_)) {
		error("Failed to encode message");
	}

	if (!sock_.send(stream_)) {
		error(SDL_GetError());
		return false;
	}

	return true;
}

bool MPClient::Impl::receive(mp_proto::ServerToClient::Reader &r)
{
	if (state_.tag != CONNECTED) {
		return false;
	}

	if (!sock_.receive(r)) {
		return false;
	}

	const char *reason = nullptr;
	if (r.isKick()) {
		reason = r.getKick().getReason().cStr();
		warn << "Kicked from server: " << reason;
		error({KICKED, reason});
		return false;
	} else if (r.isShutdown()) {
		reason = r.getShutdown().getReason().cStr();
		warn << "Server shut down: " << reason;
		error({SHUTDOWN, reason});
		return false;
	} else if (r.isPing()) {
		auto mb = builder();
		auto root = mb.initRoot<mp_proto::ClientToServer>();
		root.setPong();
		if (!sock_.encodeAndSend(mb, stream_)) {
			error("Failed to send ping message");
			return false;
		}

		// The caller should never see the ping message.
		// Pretend it didn't happen and try again.
		return receive(r);
	}

	return true;
}

void MPClient::Impl::tickInitiateConnection()
{
	if (!addr_) {
		addr_.reset(NET_ResolveHostname(opts_.host.c_str()));
		if (!addr_) {
			(warn
				<< "Failed to resolve hostname '" << opts_.host
				<< "': " << SDL_GetError());
			error(SDL_GetError());
			return;
		}
	}

	NET_Status status = NET_GetAddressStatus(addr_.get());
	if (status == NET_FAILURE) {
		warn << "Failed to resolve hostname '" << opts_.host << "'";
		error(SDL_GetError());
		return;
	} else if (status == NET_WAITING) {
		state_.reason = "Looking up hostname...";
		return;
	}

	if (!sock_) {
		sock_.reset(NET_CreateClient(addr_.get(), opts_.port, 0));
		if (!sock_) {
			warn << "Failed to connect: " << SDL_GetError();
			error(SDL_GetError());
			return;
		}
	}

	status = NET_GetConnectionStatus(sock_.get());
	if (status == NET_FAILURE) {
		warn << "Failed to connect to '" << opts_.host << "': " << SDL_GetError();
		error(SDL_GetError());
		return;
	} else if (status == NET_WAITING) {
		state_.reason = "Connecting...";
		return;
	}

	{ // Send client hello
		auto mb = builder();
		auto root = mb.initRoot<mp_proto::ClientToServer>();
		auto hello = root.initHello();
		hello.setNick(opts_.nick);
		hello.setHost(opts_.host);
		hello.setIdentifier(opts_.identifier);
		hello.setRequestWorld(opts_.requestWorld);
		if (!sock_.encode(mb, stream_)) {
			error("Failed to encode client hello");
			return;
		}

		if (!sock_.send(stream_)) {
			error(SDL_GetError());
			return;
		}
	}

	state_ = {HANDSHAKING};
}

void MPClient::Impl::tickHandshake()
{
	mp_proto::ServerToClient::Reader r;
	if (!sock_.receive(r)) {
		return;
	}

	if (r.hasShutdown()) {
		const char *reason = r.getShutdown().getReason().cStr();
		warn << "Server shut down during handshake: " << reason;
		error({SHUTDOWN, reason});
		return;
	}

	if (r.hasKick()) {
		const char *reason = r.getKick().getReason().cStr();
		warn << "Kicked by server during handshake: " << reason;
		error({KICKED, reason});
		return;
	}

	if (!r.hasHello()) {
		warn << "Received non-hello message during handshake:";
		warn << r.toString().flatten().cStr();
		error("Unexpected message during handshake");
		return;
	}

	// There's nothing interesting in the server hello message,
	// but the fact that we received it means that we're properly connected
	info << "Server handshake completed.";
	state_ = {CONNECTED};
}

void MPClient::Impl::error(State newState)
{
	if (newState.tag == ERROR) {
		timer_ = 2;
	} else if (newState.tag == SHUTDOWN) {
		timer_ = 4;
	} else {
		timer_ = 0;
	}

	state_ = std::move(newState);
	addr_.reset();
	sock_.reset();
}

MPClient::MPClient() = default;
MPClient::~MPClient() = default;

void MPClient::connect(Options opts)
{
	impl_ = std::make_unique<Impl>(std::move(opts));
}

void MPClient::end()
{
	if (impl_) {
		impl_->end();
	}

	impl_.reset();
}

void MPClient::tick(float dt)
{
	if (impl_) {
		impl_->tick(dt);
	}
}

MPClient::State MPClient::state()
{
	if (!impl_) {
		return {CLOSED};
	}

	return impl_->state();
}

bool MPClient::receive(mp_proto::ServerToClient::Reader &r)
{
	if (!impl_) {
		return false;
	}

	return impl_->receive(r);
}

}
