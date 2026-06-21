#include "rle.h"

#include <cstring>

namespace Swan {

void rleEncode8(std::vector<uint8_t> &out, std::span<const uint8_t> in)
{
	if (in.size() == 0) {
		return;
	}

	size_t len = 0;
	for (size_t i = 1; i < in.size(); ++i) {
		len += 1;
		bool encode =
			(in[i - 1] != in[i]) ||
			(len == 256);
		if (!encode) {
			continue;
		}

		out.push_back(in[i - 1]);
		out.push_back(len - 1);
		len = 0;
	}

	len += 1;
	out.push_back(in[in.size() - 1]);
	out.push_back(len - 1);
}

void rleEncode16SRO(std::vector<uint8_t> &out, std::span<const uint16_t> in)
{
	if (in.size() == 0) {
		return;
	}

	size_t len = 0;
	for (size_t i = 1; i < in.size(); ++i) {
		len += 1;
		bool encode =
			(in[i - 1] != in[i]) ||
			(len == 257);
		if (!encode) {
			continue;
		}

		if (len == 1) {
			uint16_t val = in[i - 1] | 0x8000;
			out.push_back(val & 0xff);
			out.push_back(val >> 8);
		} else {
			uint16_t val = in[i - 1] & ~0x8000;
			out.push_back(val & 0xff);
			out.push_back(val >> 8);
			out.push_back(len - 2);
		}
		len = 0;
	}

	len += 1;
	if (len == 1) {
		uint16_t val = in[in.size() - 1] | 0x8000;
		out.push_back(val & 0xff);
		out.push_back(val >> 8);
	} else if (len > 1) {
		uint16_t val = in[in.size() - 1] & ~0x8000;
		out.push_back(val & 0xff);
		out.push_back(val >> 8);
		out.push_back(len - 2);
	}
}

size_t rleDecode8(std::span<uint8_t> out, std::span<const uint8_t> in)
{
	size_t outidx = 0;
	size_t inidx = 0;
	while (inidx < in.size() - 1) {
		uint8_t val = in[inidx++];
		size_t len = in[inidx++] + 1;

		// Happy path: just write the data
		if (outidx + len <= out.size()) {
			memset(&out[outidx], val, len);
			outidx += len;
			continue;
		}

		// Sad path: fill the rest with 'val', then increment outidx
		if (outidx < out.size()) {
			memset(&out[outidx], val, out.size() - outidx);
		}
		outidx += len;
	}

	return outidx;
}

size_t rleDecode16SRO(std::span<uint16_t> out, std::span<const uint8_t> in)
{
	size_t outidx = 0;
	size_t inidx = 0;
	while (inidx < in.size() - 1) {
		uint16_t lo = in[inidx++];
		uint16_t hi = in[inidx++];
		uint16_t val = lo | (hi << 8);
		size_t len;
		if (val & 0x8000) {
			val &= ~0x8000;
			len = 1;
		} else {
			len = size_t(in[inidx++]) + 2;
		}

		// Happy path: just write the data
		if (outidx + len <= out.size()) {
			for (size_t i = 0; i < len; ++i) {
				out[outidx++] = val;
			}
			continue;
		}

		// Sad path: fill the rest with 'val', then increment outidx
		if (outidx < out.size()) {
			for (size_t i = outidx; i < out.size(); ++i)
			while (outidx < out.size()) {
				out[i] = val;
			}
		}
		outidx += len;
	}

	return outidx;
}

}
