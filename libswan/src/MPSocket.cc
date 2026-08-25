#include "MPSocket.h"
#include "kj/io.h"

#include <cstdlib>
#include <swan/log.h>

namespace Swan {

static bool enableDebug() {
	static bool enable = [] {
		const char *s = getenv("SWAN_DEBUG_NET");
		return s && std::string_view(s) == "1";
	}();
	return enable;
}

bool MPSocket::encode(capnp::MessageBuilder &mb, kj::VectorOutputStream &stream)
{
	stream.clear();

	// Reserve space for header, then write the message
	unsigned char headPlaceholder[HEADER_SIZE] = {0};
	stream.write(headPlaceholder, sizeof(headPlaceholder));
	capnp::writePackedMessage(stream, mb);

	auto out = stream.getArray();
	size_t size = out.size() - HEADER_SIZE;
	if (size > 0xffffffffull) {
		warn << "Attempt to send oversized message: " << size;
		return false;
	}

	if (enableDebug()) {
		info << "Net: Encoded " << size << " byte payload";
	}

	// Fill in header
	static_assert(HEADER_SIZE == 4);
	auto head = stream.getArray();
	head[0] = uint8_t((size & 0x000000ffull) >> 0);
	head[1] = uint8_t((size & 0x0000ff00ull) >> 8);
	head[2] = uint8_t((size & 0x00ff0000ull) >> 16);
	head[3] = uint8_t((size & 0xff000000ull) >> 24);
	return true;
}

bool MPSocket::send(const kj::VectorOutputStream &stream)
{
	// We need the output array,
	// but we don't need to modify it.
	// It should be fine to cast away const here.
	// We want to keep the type const in the signature for documentation reasons.
	// Must clean up the scratch vector before the next tx
	auto out = const_cast<kj::VectorOutputStream &>(stream).getArray();

	bool ok = NET_WriteToStreamSocket(
		sock_.get(), &out.front(), out.size());
	if (!ok) {
		warn << "Failed to send message: " << SDL_GetError();
		return false;
	}

	return true;
}

bool MPSocket::encodeAndSend(capnp::MessageBuilder &mb, kj::VectorOutputStream &stream)
{
	if (!encode(mb, stream)) {
		return false;
	}

	return send(stream);
}

bool MPSocket::receiveRaw()
{
	int n = NET_ReadFromStreamSocket(sock_.get(), rxBuf_.data() + rxIndex_, rxPending_);
	if (n <= 0) {
		return false;
	}

	// Are we done reading yet?
	rxPending_ -= n;
	rxIndex_ += n;
	if (rxPending_ > 0) {
		return false;
	}

	// Take care of reading the header
	if (rxHeader_) {
		uint32_t len = (
			(uint32_t(rxBuf_[0]) << 0) |
			(uint32_t(rxBuf_[1]) << 8) |
			(uint32_t(rxBuf_[2]) << 16) |
			(uint32_t(rxBuf_[3]) << 24));
		if (len > 0xffffffull) {
			warn << "Attempt to receive oversized message: " << len;
			warn << "Killing socket.";
			sock_.reset();
			return false;
		}

		rxIndex_ = 0;
		rxPending_ = len;
		if (rxBuf_.size() < rxPending_) {
			rxBuf_.resize(rxPending_);
		}

		rxHeader_ = false;

		// There's likely already a message in the SDL rx buffer,
		// let's read
		return receiveRaw();
	}

	// Prepare for reading a header next iteration
	size_t size = rxIndex_ + rxPending_;
	rxPending_ = HEADER_SIZE;
	rxIndex_ = 0;
	rxHeader_ = true;

	// We keep around an ArrayInputStream and a PackedMessageReader
	// so that the ServerToClient::Reader can out-live this stack frame.
	scratch_.emplace(kj::ArrayPtr{rxBuf_.data(), size});

	if (enableDebug()) {
		info << "Net: Received " << size << " byte payload";
	}

	return true;
}

}
