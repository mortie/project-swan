#pragma once

#include <optional>
#include <swan/util.h>
#include <SDL3_net/SDL_net.h>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <kj/io.h>

#include "kj/vector.h"
#include "multiplayer.capnp.h"

namespace Swan {

class MPSocket {
public:
	MPSocket(): rxBuf_(rxPending_) {}

	NET_StreamSocket *get() { return sock_.get(); }
	operator bool() const { return sock_.get(); }

	static bool encode(capnp::MessageBuilder &mb, kj::VectorOutputStream &stream);
	bool send(const kj::VectorOutputStream &stream);
	bool encodeAndSend(capnp::MessageBuilder &mb, kj::VectorOutputStream &stream);

	template<typename Reader>
	bool receive(Reader &r)
	{
		if (!receiveRaw()) {
			return false;
		}

		r = scratch_->reader.getRoot<typename Reader::Reads>();
		return true;
	}

	void reset(NET_StreamSocket *ss = nullptr)
	{
		sock_.reset(ss);
		rxPending_ = HEADER_SIZE;
		rxIndex_ = 0;
		rxHeader_ = true;
	}

	void drain(int timeout)
	{ NET_WaitUntilStreamSocketDrained(sock_.get(), timeout); }

private:
	static constexpr size_t HEADER_SIZE = 4;

	bool receiveRaw();

	struct Scratch {
		Scratch(kj::ArrayPtr<const kj::byte> array):
			stream(array),
			reader(stream)
		{}

		kj::ArrayInputStream stream;
		capnp::PackedMessageReader reader;
	};

	CPtr<NET_StreamSocket, NET_DestroyStreamSocket> sock_;

	// State for keeping track of receiving
	bool rxHeader_ = true;
	size_t rxPending_ = HEADER_SIZE;
	size_t rxIndex_ = 0;
	std::vector<unsigned char> rxBuf_;

	std::optional<Scratch> scratch_;
};

}
