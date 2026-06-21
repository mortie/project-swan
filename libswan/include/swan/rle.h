#pragma once

#include <cstdint>
#include <cstdlib>
#include <span>
#include <vector>

namespace Swan {

/**
 * Run-length-encode an array of bytes.
 * Outputs a sequence of runs, each consisting of one length byte and one value byte.
 * The length byte contains the number of sequential value bytes, minus one;
 * so a run of length 1 would be encoded with a length byte of 0.
 */
void rleEncode8(std::vector<uint8_t> &out, std::span<const uint8_t> in);

/**
 * Run-length-encode an array of 16-bit ints in the range 0..2^15.
 * Outputs a sequence of short-run-optimized runs.
 * A run can consist of either a normal RLE run:
 * - Two bytes: little endian value, with the high bit unset
 * - One byte: length of the run minus 2
 * Or a special "short run", representing a run of length 1:
 * - Two bytes: little endian value, with the high bit set
 */
void rleEncode16SRO(std::vector<uint8_t> &out, std::span<const uint16_t> in);

/**
 * Decode a buffer encoded by rleEncode8.
 * Returns the number of bytes placed in 'out' which would have been put there
 * if there was enough space.
 */
size_t rleDecode8(std::span<uint8_t> out, std::span<const uint8_t> in);

/**
 * Decode a buffer encoded by rleEncode16SRO.
 * Returns the number of bytes placed in 'out' which would have been put there
 * if there was enough space.
 */
size_t rleDecode16SRO(std::span<uint16_t> out, std::span<const uint8_t> in);

}
